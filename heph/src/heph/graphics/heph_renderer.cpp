#include "heph_renderer.hpp"

#include "utils/heph_error.hpp"
#include "log/heph_log.hpp"
#include "utils/heph_utils.hpp"
#include "heph_thread_pool.hpp"
#include "types/heph_type_string.hpp"
#include "types/heph_type_vertex.hpp"
#include "../bootstrap/VkBootstrap.cpp"

#include <shaderc/shaderc.hpp>
#include <stdint.h>

void heph_renderer_backend_init(heph_renderer_t *const r, char *const window_name, int width, int height, heph_thread_pool_t *const application_thread_pool)
{
        heph_renderer_init_instance(r);
        r->window_width = width;
        r->window_height = height;

        /* Vulkan Core Constructs */
        heph_renderer_init_window(r, window_name);
        heph_renderer_init_surface(r);
        heph_renderer_init_pdevice(r);
        heph_renderer_find_memory_type_indices(r);
        heph_renderer_init_ldevice(r);
        heph_renderer_init_queue(r);
        heph_renderer_init_swapchain(r);
        heph_renderer_aquire_swapchain_images(r);

        /* Engine Specifics */
        heph_renderer_init_command_pools(r);
        heph_renderer_init_frame_render_infos(r);
        heph_renderer_init_sync_structures(r);
        heph_renderer_init_graphics_pipelines(r);
        heph_renderer_init_compute_pipelines(r);
        heph_renderer_allocate_engine_buffers(r);

}

void heph_renderer_init_instance(heph_renderer_t *const r)
{
        vkb::InstanceBuilder builder;
        auto res = builder
                       .set_app_name("Hephaestus Renderer")
                       .set_engine_name("Hephaestus Engine")
                       .require_api_version(1, 3, 0)
                       .use_default_debug_messenger()
                       //        .set_debug_callback(debug_callback)
                       .request_validation_layers()
                       .build();
        HEPH_COND_ABORT((bool)res, false);
        r->vkb_instance = res.value();
        r->instance = r->vkb_instance.instance;
}

void heph_renderer_init_window(heph_renderer_t *const r, char *const name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        HEPH_COND_ABORT((r->window = glfwCreateWindow(r->window_width, r->window_height, name, NULL, NULL)), NULL);
}

void heph_renderer_init_surface(heph_renderer_t *const r)
{
        HEPH_COND_ABORT_NE(glfwCreateWindowSurface(r->instance, r->window, NULL, &r->surface), VK_SUCCESS);
}

void heph_renderer_init_pdevice(heph_renderer_t *const r)
{
        printf("\n\n*** DONT DELETE THIS PRINTF UNTIL YOU FIX THE DAMN PICK FIRST DEVICE UNCONDITIONALLY. ***\n\n");
        vkb::PhysicalDeviceSelector selector(r->vkb_instance);
        auto res = selector
                       .set_surface(r->surface)
                       .select_first_device_unconditionally()
                       .select();
        HEPH_COND_ABORT((bool)res, false);
        r->vkb_pdevice = res.value();
        r->pdevice = r->vkb_pdevice.physical_device;
}

void heph_renderer_find_memory_type_indices(heph_renderer_t *const r)
{
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(r->pdevice, &physical_device_memory_properties);

        /* TODO
                once the engine buffers memory types are different from eachother you must fix this
        */
#warning once the engine buffers memory types are different from eachother you must fix this
#if HEPH_RENDERER_GEOMETRY_BUFFER_MEMORY_TYPE_BITFLAGS == HEPH_RENDERER_REQUIRED_OBJECT_BUFFER_MEMORY_TYPE_BITFLAGS == HEPH_RENDERER_REQUIRED_DRAW_BUFFER_MEMORY_TYPE_BITFLAGS
        for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
        {
                if ((HEPH_RENDERER_REQUIRED_GEOMETRY_BUFFER_BITFLAGS & physical_device_memory_properties.memoryTypes[i]) == HEPH_RENDERER_REQUIRED_GEOMETRY_BUFFER_BITFLAGS)
                {
                        r->geometry_buffer_memory_type_index = i;
                        r->object_buffer_memory_type_index = i;
                        r->draw_buffer_memory_type_index = i;

                        return;
                }
        }
#endif

        /* TODO do some fallback like required a staging buffer */
        HEPH_ABORT("Unable to find suitable memory types for engine buffers.");
}

void heph_renderer_init_ldevice(heph_renderer_t *const r)
{
        VkPhysicalDeviceSynchronization2Features sync_features = {};
        sync_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync_features.synchronization2 = VK_TRUE;

        vkb::DeviceBuilder builder{r->vkb_pdevice};
        auto res = builder
                       .add_pNext(&sync_features)
                       .build();
        HEPH_COND_ABORT((bool)res, false);
        r->vkb_ldevice = res.value();
        r->ldevice = r->vkb_ldevice.device;
}

void heph_renderer_init_queue(heph_renderer_t *const r)
{
        uint32_t i = 0;
        for (VkQueueFamilyProperties props : r->vkb_pdevice.get_queue_families())
        {
                VkQueueFlags flags = props.queueFlags;
                if ((flags & REQUIRED_QUEUE_FAMILY_BITFLAGS) == REQUIRED_QUEUE_FAMILY_BITFLAGS)
                {
                        goto found;
                }
                i++;
        }
        HEPH_ABORT("Unable to find suitable queue for Hephaestus to use");
found:
        r->queue_family_index = i;
        vkGetDeviceQueue(r->ldevice, i, 0, &r->queue);
}

void heph_renderer_init_swapchain(heph_renderer_t *const r)
{
        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder
                       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .build();
        HEPH_COND_ABORT((bool)res, false);
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;
}

void heph_renderer_aquire_swapchain_images(heph_renderer_t *const r)
{
        HEPH_COND_ABORT_NE(vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->swapchain_nimages, NULL), VK_SUCCESS);

        r->swapchain_images = (VkImage *)calloc(sizeof(VkImage), r->swapchain_nimages);

        HEPH_COND_ABORT_NE(vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->swapchain_nimages, r->swapchain_images), VK_SUCCESS);

#if HEPH_VALIDATE
        for (uint32_t i = 0; i < r->swapchain_nimages; i++)
        {
                printf("Swapchain image %u handle: %p\n", i, r->swapchain_images[i]);
        }
#endif
}

void heph_renderer_init_command_pools(heph_renderer_t *const r)
{
        /* Main command pool */
        VkCommandPoolCreateInfo main_command_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = r->queue_family_index,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};

        HEPH_COND_ABORT_NE(vkCreateCommandPool(r->ldevice, &main_command_pool_create_info, NULL, &r->main_command_pool), VK_SUCCESS);

        /* Recording thread command pool */
        VkCommandPoolCreateInfo command_buffer_recording_command_pool = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = r->queue_family_index};

        VkCommandPool recording_thread_command_pool = VK_NULL_HANDLE;
        HEPH_COND_ABORT_NE(vkCreateCommandPool(r->ldevice, &command_buffer_recording_command_pool, NULL, &r->command_buffer_recording_command_pool), VK_SUCCESS);
}

void heph_renderer_init_sync_structures(heph_renderer_t *const r)
{
        /* Image acquired */
        VkSemaphoreCreateInfo image_acquired_semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        HEPH_COND_ABORT_NE(vkCreateSemaphore(r->ldevice, &image_acquired_semaphore_info, NULL, &r->image_acquired_semaphore), VK_SUCCESS);
}

void heph_renderer_init_frame_render_infos(heph_renderer_t *const r)
{
        r->prev_resource_index = UINT32_MAX;
        r->frame_render_infos = (HephFrameRenderInfos *)HCALLOC(r->swapchain_nimages, sizeof(HephFrameRenderInfos));

        VkFenceCreateInfo frame_render_infos_render_complete_fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT};

        VkSemaphoreCreateInfo frame_render_infos_render_complete_semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        VkCommandBufferAllocateInfo frame_render_infos_command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = r->command_buffer_recording_command_pool,
            .commandBufferCount = 1,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY};

        for (uint32_t i = 0; i < r->swapchain_nimages; i++)
        {
                HEPH_COND_ABORT_NE(vkCreateFence(r->ldevice, &frame_render_infos_render_complete_fence_info, NULL, &r->frame_render_infos[i].render_complete_fence), VK_SUCCESS);
                HEPH_COND_ABORT_NE(vkCreateSemaphore(r->ldevice, &frame_render_infos_render_complete_semaphore_info, NULL, &r->frame_render_infos[i].render_complete_semaphore), VK_SUCCESS);
                HEPH_COND_ABORT_NE(vkAllocateCommandBuffers(r->ldevice, &frame_render_infos_command_buffer_allocate_info, &r->frame_render_infos[i].command_buffer), VK_SUCCESS);
        }
}

/*
        TODO
        once we add hot reloading the performance of this might actually matter.

        Frees the original text, as it is unneeded.
*/
void heph_renderer_preprocess_vertex_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->sb, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_preprocess_fragment_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->sb, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_compile_vertex_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_spv(compiler, src->ptr, src->sb, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_compile_fragment_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_spv(compiler, src->ptr, src->sb, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_init_shader_compiler(heph_renderer_t *const r)
{
        r->shader_compiler = shaderc_compiler_initialize();
        HEPH_COND_ABORT(r->shader_compiler, NULL);
}

void heph_renderer_init_shader_modules(heph_renderer_t *const r)
{
        /*
                TODO
                maybe add something where you traverse the users project directory, and find shaders
                or maybe have a builtin shader folder

                make a config file thing with options like bloom and stuff then add macro defines
                based off the config
        */

        /* Init shader modules for builtin engine shaders */
        /* Vertex shader*/
        heph_string_t vertex_shader_src = {};
        HEPH_COND_ABORT_NE(heph_file_read_to_string(&vertex_shader_src, "shader/vertex.vert"), true);
        heph_renderer_preprocess_vertex_shader(r->shader_compiler, &vertex_shader_src);
        heph_renderer_compile_vertex_shader(r->shader_compiler, &vertex_shader_src);

        VkShaderModuleCreateInfo vertex_shader_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = vertex_shader_src.sb,
            .pCode = (uint32_t *)vertex_shader_src.ptr};

        /* Fragment shader */
        heph_string_t fragment_shader_src = {};
        HEPH_COND_ABORT_NE(heph_file_read_to_string(&fragment_shader_src, "shader/fragment.frag"), true);
        heph_renderer_preprocess_fragment_shader(r->shader_compiler, &fragment_shader_src);
        heph_renderer_compile_fragment_shader(r->shader_compiler, &fragment_shader_src);

        VkShaderModuleCreateInfo fragment_shader_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = fragment_shader_src.sb,
            .pCode = (uint32_t *)fragment_shader_src.ptr};
}

void heph_renderer_init_graphics_pipelines(heph_renderer_t *const r)
{
        r->npipelines = 1;

        /* Pipeline rendering create info */
        VkFormat color_attachment_format = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .depthAttachmentFormat = VK_FORMAT_R32G32B32_SFLOAT,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format,
            .stencilAttachmentFormat = VK_FORMAT_R32G32B32_SFLOAT,
            .viewMask = 0};

        /* Grahpics pipeline create info */
        uint32_t ntotal_shaders = r->nadditional_shader_modules + HEPH_RENDERER_NBUILTIN_SHADER_STAGES;
        VkPipelineShaderStageCreateInfo *shader_stage_create_infos = (VkPipelineShaderStageCreateInfo *)HCALLOC(ntotal_shaders, sizeof(VkPipelineShaderStageCreateInfo));
        /* Vertex shader*/
        shader_stage_create_infos[0] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = r->vertex_shader_module,
            .pName = "main"};
        /* Fragment shader */
        shader_stage_create_infos[1] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = r->fragment_shader_module,
            .pName = "main"};

        /* Vertex input state infos */
        VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2] = {};
        VkVertexInputBindingDescription vertex_input_binding_descriptions[2] = {};

        /* Vertex input attribute descriptions */
        /* Position */
        vertex_input_attribute_descriptions[0] = {
            .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .location = 0,
            .offset = offsetof(Vertex, pos)};
        /* Normal */
        vertex_input_attribute_descriptions[1] = {
            .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .location = 0,
            .offset = offsetof(Vertex, normal)};

        /* Vertex input binding descriptions */
        /* Position */
        vertex_input_binding_descriptions[0] = {
            .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
            .stride = sizeof(Vec3),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
        /* Normal */
        vertex_input_binding_descriptions[1] = {
            .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL,
            .stride = sizeof(Vec3),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 2,
            .pVertexBindingDescriptions = vertex_input_binding_descriptions,
            .vertexAttributeDescriptionCount = 2,
            .pVertexAttributeDescriptions = vertex_input_attribute_descriptions,
        };

        /* Input assembly state info */
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .vertexBindingDescriptionsCount = 2,
            .pVertexBindingDescriptions = vertex_input_binding_descriptions,
            .vertexAttributeDescriptionsCount = 2,
            .pVertexAttributeDescriptions = vertex_input_attribute_descriptions};

        /* Tessellation */
        VkPipelineTessellationStateCreateInfo tessellation_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pathControlPoints = 0};

        /* Viewport state */
        VkViewport viewport = {
            .x = 0.0,
            .y = 0.0,
            .width = 1920.0,
            .height = 1080.0,
            .minDepth = 0.0,
            .maxDepth = 1.0};

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = {1920, 1080}};

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .flags =,
            .viewportCount = HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor};

        /* Rasterization State */
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .flags =,
            .depthClampEnable =,
            .rasterizerDiscardEnable =,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode =,
            .frontFace =,
            .depthBiasEnable =,
            .depthBiasConstantFactor =,
            .depthBiasClamp =,
            .deptphBiasSlopeFactor =,
            .lineWidth = 1.0};

        /* Multisample state */
        VkPipelineMultisampleStateCreateInfo multisample_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .flags = };

        /* Final grahpics pipeline */
        VkGraphicsPipelineCreateInfo pipeline_create_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipeline_rendering_create_info,
            .flags =,
            .stageCount = 2,
            .pStages = shader_stage_create_infos,
            .pVertexInputState = &vertex_input_state_create_info,
            .pInputAssemblyState = input_assembly_state_create_info,

        };

        VkPipeline pipeline;
        vkCreateGraphicsPipelines(r->ldevice, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline);
}

void heph_renderer_preprocess_compute_shader(heph_renderer_t *const r)
{
}

void heph_renderer_compile_compute_shader(heph_renderer_t *const r, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_spv(compiler, src->ptr, src->sb, shaderc_compute_shader, "compute_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_init_compute_pipelines(heph_renderer_t *const r)
{
        heph_string_t compute_shader_src = {};
        HEPH_COND_ABORT_NE(heph_file_read_to_string(&compute_shader_src, "shader/compute.comp"), true);
        heph_renderer_preprocess_compute_shader(r->shader_compiler, &compute_shader_src);
        heph_renderer_compile_compute_shader(r->shader_compiler, &compute_shader_src);

        /* Shader Module */
        VkShaderModuleCreateInfo compute_shader_module_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .flags =,
            .codeSize =,
            .pCode = };

        VkShaderModule compute_shader_module = {};
        vkCreateShaderModule(r->ldevice, &compute_shader_module_create_info, NULL, &compute_shader_module);

        /* Shader stage */
        VkPipelineShaderStageCreateInfo compute_shader_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = compute_shader_module,
            .pName = "main"};

        /* Push constant range */
        const uint32_t frustum_size = sizeof(float) * 32;
        const uint32_t object_buffer_swap_size = sizeof(uint);

        VkPushConstantRange push_constant_range = {
                .stageFlags = VK_SHADER_COMPUTE_BIT,
                .offset = 0,
                .size = frustum_size + object_buffer_swap_size;
};

/* Descriptor sets */
VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[];

/* Object buffer */
descriptor_set_layout_bindings[0] = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .descriptorCount = 1,
    .stageFlags =,
    .pImmutableSamplers = };

VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT,
    .flags =,
    .bindingCount =,
    .pBinding};

VkDescriptorSetLayout descriptor_set_layout = {};
vkCreateDescriptorSetLayout(r->ldevice, &descriptor_set_layout_create_info, NULL, &descriptor_set_layout);

/* Pipeline layout */
VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info = {
    .sType = VK_PIPELINE_LAYOUT_CREATE_INFO,
    .flags =,
    .setLayoutCount =,
    .pSetLayouts =,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &compute_pipeline_layout_push_constant_range};

vkCreatePipelineLayout(r->ldevice, &compute_pipeline_layout_create_info, NULL, &r->compute_pipeline_layout);

/* Create compute pipeline */
VkComputePipelineCreateInfo compute_pipeline_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .stage = compute_shader_stage_create_info,
    .layout =,
    .basePipelineHandle =,
    .basePipelineIndex = };

vkCreateComputePipelines(r->ldevice, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, NULL, &r->compute_pipeline);
}

void heph_renderer_allocate_engine_buffers(heph_renderer_t *const r)
{
        /* Initialize geometry buffer */
        uint32_t geometry_buffer_size_bytes = ;

        VkBufferCreateInfo geometry_buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = geometry_buffer_size_bytes,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        HEPH_COND_ABORT_NE(vkCreateBuffer(r->ldevice, &geometry_buffer_create_info, NULL, r->geometry_buffer), VK_SUCCESS);

        VkMemoryAllocateInfo geometry_buffer_memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = geometry_buffer_size_bytes,
            .memoryTypeIndex = r->geometry_buffer_memory_type_index};

        vkAllocateMemory(r->ldevice, NULL, NULL, &r->geometry_buffer_device_memory);

        /* Initialize object buffer */
        r->object_buffer_swap = 0;
        uint32_t object_buffer_size_bytes = 2 * nobjects;

        VkBufferCreateInfo object_buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = object_buffer_size_bytes,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};

        HEPH_COND_ABORT_NE(vkCreateBuffer(r->ldevice, &object_buffer_create_info, NULL, r->object_buffer), VK_SUCCESS);

        VkMemoryAllocateInfo object_buffer_memory_allocate_info = {
            .sType VK_STRUCTURE_TYPE_MEMORTY_ALLOCATE_INFO,
            .allocationSize = object_buffer_size_bytes,
            .memoryTypeIndex = r->object_buffer_memory_type_index};

        vkAllocateMemory(r->ldevice, NULL, NULL, &r->object_buffer_device_memory);

        /* Initialize draw buffer */
        uint32_t draw_buffer_size_bytes = nobjects * sizeof(VkDrawIndexedIndirectCommand);

        VkBufferCreateInfo draw_buffer_create_info = {
            .sType VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size =,
            .usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};

        HEPH_COND_ABORT_NE(vkCreateBuffer(r->ldevice, &draw_buffer_create_info, NULL, r->draw_buffer), VK_SUCCESS);

        VkMemoryAllocateInfo draw_buffer_memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = object_buffer_size,
            .memoryTypeIndex = r->draw_buffer_memory_type_index};
}

void heph_renderer_rebuild_swapchain(heph_renderer_t *const r, int width, int height)
{
        /*
                TODO
                using vkb for the swapchain recreation might not be good i dont know
        */

        vkDeviceWaitIdle(r->ldevice);

        vkb::destroy_swapchain(r->vkb_swapchain);

        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder.build();
        if (!res)
        {
                HEPH_ABORT("recreate swapchain: " << res.error().message());
        }
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;

        heph_renderer_aquire_swapchain_images(r);
}

void heph_renderer_handle_window_resize(heph_renderer_t *const r, int width, int height)
{
        r->window_width = width;
        r->window_height = height;
        heph_renderer_rebuild_swapchain(r, width, height);
        heph_renderer_recalculate_projection_matrix(r, );
}

void heph_renderer_render(heph_renderer_t *const r)
{
        for (uint32_t s = 0; s < r->nscenes; s++)
        {
                r->current_scene = &r->scenes[s];

                for (uint32_t c = 0; c < r->current_scene->ncameras; c++)
                {
                        r->current_camera = &r->current_scene->cameras[c];

                        heph_renderer_render_frame(r);
                }
        }
}

/* Renders a single frame */
void heph_renderer_render_frame(heph_renderer_t *const r)
{
        heph_scene_t *scene = r->current_scene;
        heph_camera_t *camera = r->current_camera;

        uint32_t resource_index = (r->resource_index + 1) * !(r->resource_index + 1 == r->swapchain_nimages);

        VkFence render_complete_fence = r->frame_render_infos[resource_index].render_complete_fence;
        VkSemaphore render_complete_semaphore = r->frame_render_infos[resource_index].render_complete_semaphore;
        VkCommandBuffer frame_command_buffer = r->frame_render_infos[resource_index].command_buffer;

        /* Make sure the fence we are going to use is not in use */
        HEPH_COND_ABORT_NE(vkWaitForFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence, VK_TRUE, HEPH_RENDERER_MAX_TIMEOUT), VK_SUCCESS);
        HEPH_COND_ABORT_NE(vkResetFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence), VK_SUCCESS);

        uint32_t image_index;
        HEPH_COND_ABORT_NE(vkAcquireNextImageKHR(r->ldevice, r->swapchain, HEPH_RENDERER_MAX_TIMEOUT, r->image_acquired_semaphore, NULL, &image_index), VK_SUCCESS);
        VkImage target_image = r->swapchain_images[image_index];

        /* Pepare the frame command buffer for recording */
        VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkResetCommandBuffer(command_buffer, 0);
        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

        /* Dispatch culling compute shader */
        vkCmdBindPipeline(frame_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, r->compute_pipeline);

        /* PushConstant the frustum and object buffer swap boolean */
        camera->__push_constant_padding = scene->object_buffer_swap;
        vkCmdPushConstants(
            frame_command_buffer,
            r->compute_pipeline_layout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(heph_camera_t),
            &r->current_camera->__push_constant_padding
        );

        vkCmdBindDescriptorSets(frame_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, r->compute_pipeline_layout, 0, 3, &r->geometry_buffer_descriptor, 0, NULL);
        vkCmdDispatch(frame_command_buffer, ceil(nobjects / 16), 1, 1);

        /* Sync access to draw buffer */
        VkBufferMemoryBarrier2 draw_buffer_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcAccessMask =,
            .dstAccessMask =,
            .srcQueueFamilyIndex =,
            .dstQueueFamilyIndex =,
            .buffer = r->draw_buffer,
            .offset = 0,
            .size = r->nobjects
        };

        /* Translate target image: UNDEFINED -> COLOR_ATTACHMENT */
        VkImageSubresourceRange target_image_subresource_range = {
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT};

        VkImageMemoryBarrier2 target_image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE, // None is sufficient, aquiring the image is synced
            .dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = target_image,
            .subresourceRange = target_image_subresource_range};

        /* Barrier for image transition UNDEFINED -> COLOR_ATTACHMENT and compute culler */
        VkDependencyInfo barriers_dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &draw_buffer_memory_barrier,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &target_image_memory_barrier};

        vkCmdPipelineBarrier2(frame_command_buffer, &barriers_dependency_info);

        /* Bind required 'graphics' resources */
        vkCmdBindVertexBuffers(frame_command_buffer, 0, 1, scene->geometry_buffer (uint32_t[]){ 0 });
        vkCmdBindIndexBuffer(frame_command_buffer, scene->geometry_buffer, scene->vertices_size_bytes, VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(frame_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

        /* PushConstant the view / projection matrices */
        // TODO this doesnt feel right...
        float vertex_shader_push_constant_data[32] = {};
        memcpy(vertex_shader_push_constant_data, r->camera.view_matrix, sizeof(float) * 16);
        memcpy(&vertex_shader_push_constant_data[16], r->projection_matrix, sizeof(float) * 16);
        vkCmdPushConstants(
            frame_command_buffer,
            r->graphics_pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(float) * 32,
            (void *)vertex_shader_push_constant_data);

        vkCmdDrawIndexedIndirect(frame_command_buffer, draw_buffer, 0, draw_buffer_nids, sizeof(VkDrawIndexedIndirectCommand));

        /* Translate target image: COLOR_ATTACHMENT -> PRESENTABLE */
        VkImageMemoryBarrier2 target_image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE, // None is sufficient, acquiring the image is synced
            .dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = target_image,
            .subresourceRange = target_image_subresource_range};

        /* Barrier for image transition COLOR_ATTACHMENT -> PRESENTABLE */
        // TODO bad name
        VkDependencyInfo presentation_dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &target_image_memory_barrier};

        vkCmdPipelineBarrier2(frame_command_buffer, &presentation_dependency_info);

        vkEndCommandBuffer(frame_command_buffer);

        /* Submit the main command buffer */
        VkCommandBufferSubmitInfoKHR main_command_buffer_submit_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = command_buffer};

        VkSemaphoreSubmitInfo frame_render_complete_semaphore_submit_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = render_complete_semaphore,
        };

        VkSemaphoreSubmitInfo submit_info_wait_semaphores_infos[2];
        submit_info_wait_semaphores_infos[0] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = r->image_acquired_semaphore};
        submit_info_wait_semaphores_infos[1] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = r->frame_render_infos[r->prev_resource_index].render_complete_semaphore};

        VkSubmitInfo2 submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &main_command_buffer_submit_info,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &frame_render_complete_semaphore_submit_info,
            .waitSemaphoreInfoCount = (uint32_t)2 - (r->prev_resource_index == UINT32_MAX),
            .pWaitSemaphoreInfos = submit_info_wait_semaphores_infos};

        HEPH_COND_ABORT_NE(vkQueueSubmit2(r->queue, 1, &submit_info, r->frame_render_infos[resource_index].render_complete_fence), VK_SUCCESS);

        /* Present the frame to the screen */
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &r->swapchain,
            .waitSemaphoreCount = 1,
            .pImageIndices = &image_index,
            .pWaitSemaphores = &render_complete_semaphore,
        };

        HEPH_COND_ABORT_NE(vkQueuePresentKHR(r->queue, &present_info), VK_SUCCESS);

        r->resource_index = resource_index;
        r->object_buffer_swap = !r->object_buffer_swap;
}

/* Use only for debug mode. The OS is faster at cleaning up. */
void heph_renderer_destroy(heph_renderer_t *const r)
{
        vkDeviceWaitIdle(r->ldevice);

        glfwDestroyWindow(r->window);
        glfwTerminate();

        /* Destroy shader modules and free src arrays*/
        vkDestroyShaderModule(r->ldevice, r->vertex_shader_module, NULL);
        vkDestroyShaderModule(r->ldevice, r->fragment_shader_module, NULL);
        vkDestroyShaderModule(r->ldevice, r->compute_shader_module, NULL);
        free(r->vertex_shader_src);
        free(r->fragment_shader_src);
        free(r->compute_shader_src);

        for (uint32_t i = 0; i < r->nadditional_shader_modules; i++)
        {
                ;
                sfsfsfsfsf
        }

        for (uint32_t i = 0; i < r->swapchain_nimages; i++)
        {
                vkDestroyFence(r->ldevice, r->frame_render_infos[i].render_complete_fence, NULL);
                vkDestroySemaphore(r->ldevice, r->frame_render_infos[i].render_complete_semaphore, NULL);
        }
        HFREE(r->frame_render_infos);

        /* Do not change ordering */
        for (uint32_t i = 0; i < r->swapchain_nimages; i++)
        {
                vkDestroyImage(r->ldevice, r->swapchain_images[i], NULL);
        }
        free(r->swapchain_images);
        vkDestroyCommandPool(r->ldevice, r->main_command_pool, NULL);
        vkFreeMemory(r->ldevice, r->data_buffer_memory, NULL);
        vkDestroyBuffer(r->ldevice, r->data_buffer, NULL);
        vkb::destroy_swapchain(r->vkb_swapchain);
        vkb::destroy_surface(r->vkb_instance, r->surface);
        vkb::destroy_device(r->vkb_ldevice);
        vkb::destroy_instance(r->vkb_instance);
}
