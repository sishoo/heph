#include "renderer.hpp"

#include "utils/heph_error.hpp"
#include "log/heph_log.hpp"
#include "utils/heph_utils.hpp"
#include "heph_thread_pool.hpp"
#include "types/heph_type_string.hpp"
#include "types/heph_type_vertex.hpp"
#include "bootstrap/VkBootstrap.cpp"



#include <shaderc/shaderc.hpp>




#include <stdint.h>


static void get_buffer_memory_requirements(VkDevice ldevice, VkBuffer buffer, VkMemoryRequirements2 &requirements)
{
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        VkBufferMemoryRequirementsInfo2 info;
        info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        info.pNext = NULL;
        info.buffer = buffer;
        vkGetBufferMemoryRequirements2(ldevice, &info, &requirements);
}

static uint32_t get_memory_type_index(VkPhysicalDevice pdevice)
{
        VkPhysicalDeviceMemoryProperties2 pdevice_properties = {};
        pdevice_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(pdevice, &pdevice_properties);

        uint32_t i = 0;
        for (VkMemoryType type : pdevice_properties.memoryProperties.memoryTypes)
        {
                if ((type.propertyFlags & REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS) == REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS)
                {
                        return i;
                }
                i++;
        }
        HEPH_ABORT("Cannot find adequate memory type.");
}

/*
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity_bits,
    VkDebugUtilsMessageTypeFlagBitsEXT message_type_bits,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data)
{

        std::cout << callback_data->pMessage << "\n\n\n";


        return VK_FALSE;
}
*/

void heph_renderer_init(HephRenderer *const r, char *const window_name, int width, int height, HephThreadPool *const application_thread_pool)
{
        r->heph_application_thread_pool = application_thread_pool;
        heph_renderer_init_instance(r);
        r->window_width = width;
        r->window_height = height;

        /* Vulkan Core Constructs */
        heph_renderer_init_window(r, window_name);
        heph_renderer_init_surface(r);
        heph_renderer_init_pdevice(r);
        heph_renderer_init_ldevice(r);
        heph_renderer_init_queue(r);
        heph_renderer_init_swapchain(r);
        heph_renderer_aquire_swapchain_images(r);

        /* Engine Specifics */
        heph_renderer_init_command_pools(r);
        heph_renderer_init_frame_render_infos(r);
        heph_renderer_init_sync_structures(r);
        // heph_renderer_init_graphics_pipelines(r);
        heph_renderer_init_compute_pipelines(r);
}

void heph_renderer_init_instance(HephRenderer *const r)
{
        /*
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};
        debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_messenger_create_info.pfnUserCallback = ;
        */

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

void heph_renderer_init_window(HephRenderer *const r, char *const name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        HEPH_COND_ABORT((r->window = glfwCreateWindow(r->window_width, r->window_height, name, NULL, NULL)), NULL);
}

void heph_renderer_init_surface(HephRenderer *const r)
{
        HEPH_COND_ABORT_NE(glfwCreateWindowSurface(r->instance, r->window, NULL, &r->surface), VK_SUCCESS);
}

void heph_renderer_init_pdevice(HephRenderer *const r)
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

void heph_renderer_init_ldevice(HephRenderer *const r)
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

void heph_renderer_init_queue(HephRenderer *const r)
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
        vkGetDeviceQueue(r->ldevice, i, 0, &(r->queue));
}

void heph_renderer_init_swapchain(HephRenderer *const r)
{
        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder
                       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .build();
        HEPH_COND_ABORT((bool)res, false);
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;
}

void heph_renderer_aquire_swapchain_images(HephRenderer *const r)
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

void heph_renderer_init_command_pools(HephRenderer *const r)
{
        /* Main command pool */
        VkCommandPoolCreateInfo main_command_pool_create_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = r->queue_family_index,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        };

        HEPH_COND_ABORT_NE(vkCreateCommandPool(r->ldevice, &main_command_pool_create_info, NULL, &r->main_command_pool), VK_SUCCESS);

        /* Recording thread command pool */
        VkCommandPoolCreateInfo command_buffer_recording_command_pool = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = r->queue_family_index
        };

        VkCommandPool recording_thread_command_pool = VK_NULL_HANDLE;
        HEPH_COND_ABORT_NE(vkCreateCommandPool(r->ldevice, &command_buffer_recording_command_pool, NULL, &r->command_buffer_recording_command_pool), VK_SUCCESS);
}

void heph_renderer_init_sync_structures(HephRenderer *const r)
{
        /* Image acquired */
        VkSemaphoreCreateInfo image_acquired_semaphore_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        HEPH_COND_ABORT_NE(vkCreateSemaphore(r->ldevice, &image_acquired_semaphore_info, NULL, &r->image_acquired_semaphore), VK_SUCCESS);
}

void heph_renderer_init_frame_render_infos(HephRenderer *const r)
{
        r->prev_resource_index = UINT32_MAX;
        r->frame_render_infos = (HephFrameRenderInfos *)HCALLOC(r->swapchain_nimages, sizeof(HephFrameRenderInfos));

        VkFenceCreateInfo frame_render_infos_render_complete_fence_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        VkSemaphoreCreateInfo frame_render_infos_render_complete_semaphore_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkCommandBufferAllocateInfo frame_render_infos_command_buffer_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = r->command_buffer_recording_command_pool,
                .commandBufferCount = 1,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
        };

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
*/

/*      
        Frees the original text, as it is unneeded.
*/
void heph_renderer_preprocess_vertex_shader(shaderc_compiler_t compiler, HephString *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();    

        shaderc_compilation_result_t result = 
                shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->sb, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

/*      
        Frees the original text, as it is unneeded.
*/
void heph_renderer_preprocess_fragment_shader(shaderc_compiler_t compiler, HephString *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();    

        shaderc_compilation_result_t result = 
                shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->sb, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}

void heph_renderer_compile_vertex_shader(shaderc_compiler_t compiler, HephString *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();    

        shaderc_compilation_result_t result = 
                shaderc_compile_into_spv(compiler, src->ptr, src->sb, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}               

void heph_renderer_compile_fragment_shader(shaderc_compiler_t compiler, HephString *const src)
{       
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();    

        shaderc_compilation_result_t result = 
                shaderc_compile_into_spv(compiler, src->ptr, src->sb, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_COND_ABORT_NE(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->sb = shaderc_result_get_length(result);
}               

void heph_renderer_init_shader_compiler(HephRenderer *const r)
{
        r->shader_compiler = shaderc_compiler_initialize();
        HEPH_COND_ABORT(r->shader_compiler, NULL);
}

void heph_renderer_init_shader_modules(HephRenderer *const r)
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
        HephString vertex_shader_src = {};
        HEPH_COND_ABORT_NE(heph_file_read_to_string(&vertex_shader_src, "shader/vertex.vert"), true);
        heph_renderer_preprocess_vertex_shader(r->shader_compiler, &vertex_shader_src);
        heph_renderer_compile_vertex_shader(r->shader_compiler, &vertex_shader_src);

        VkShaderModuleCreateInfo vertex_shader_create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = vertex_shader_src.sb,
                .pCode = (uint32_t *)vertex_shader_src.ptr
        };

        /* Fragment shader */
        HephString fragment_shader_src = {};
        HEPH_COND_ABORT_NE(heph_file_read_to_string(&fragment_shader_src, "shader/fragment.frag"), true);
        heph_renderer_preprocess_fragment_shader(r->shader_compiler, &fragment_shader_src);
        heph_renderer_compile_fragment_shader(r->shader_compiler, &fragment_shader_src);

        VkShaderModuleCreateInfo fragment_shader_create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = fragment_shader_src.sb,
                .pCode = (uint32_t *)fragment_shader_src.ptr
        };        
}

void heph_renderer_init_graphics_pipelines(HephRenderer *const r)
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
                .viewMask = 0
        };

        /* Grahpics pipeline create info */
        uint32_t ntotal_shaders = r->nadditional_shader_modules + HEPH_RENDERER_NBUILTIN_SHADER_STAGES;
        VkPipelineShaderStageCreateInfo *shader_stage_create_infos = (VkPipelineShaderStageCreateInfo *)HCALLOC(ntotal_shaders, sizeof(VkPipelineShaderStageCreateInfo));
        /* Vertex shader*/
        shader_stage_create_infos[0] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = r->vertex_shader_module,
                .pName = "main"
        };
        /* Fragment shader */
        shader_stage_create_infos[1] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = r->fragment_shader_module,
                .pName = "main"
        };


        /* Vertex input state infos */
        VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2] = {};
        VkVertexInputBindingDescription vertex_input_binding_descriptions[2] = {};
       
        /* Vertex input attribute descriptions */
        /* Position */
        vertex_input_attribute_descriptions[0] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .location = 0,
                .offset = offsetof(Vertex, pos)
        };      
        /* Normal */
        vertex_input_attribute_descriptions[1] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .location = 0,
                .offset = offsetof(Vertex, normal)
        };

        /* Vertex input binding descriptions */
        /* Position */
        vertex_input_binding_descriptions[0] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION,
                .stride = sizeof(Vec3),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        /* Normal */
        vertex_input_binding_descriptions[1] = {
                .binding = HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL,
                .stride = sizeof(Vec3),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

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
                .pVertexAttributeDescriptions = vertex_input_attribute_descriptions
        };

        /* Tessellation */
        VkPipelineTessellationStateCreateInfo tessellation_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                .pathControlPoints = 0
        };

        /* Viewport state */
        VkViewport viewport = {
                .x = 0.0,
                .y = 0.0,
                .width = 1920.0,
                .height = 1080.0,
                .minDepth = 0.0,
                .maxDepth = 1.0
        };

        VkRect2D scissor = {
                .offset = {0, 0},
                .extent = {1920, 1080}
        };

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .flags = ,
                .viewportCount = HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor
        };

        /* Rasterization State */
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .flags = ,
                .depthClampEnable = ,
                .rasterizerDiscardEnable = ,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = ,
                .frontFace = ,
                .depthBiasEnable = ,
                .depthBiasConstantFactor = ,
                .depthBiasClamp = ,
                .deptphBiasSlopeFactor = ,
                .lineWidth = 1.0
        };

        /* Multisample state */
        VkPipelineMultisampleStateCreateInfo multisample_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .flags = 
        };

        /* Final grahpics pipeline */
        VkGraphicsPipelineCreateInfo pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &pipeline_rendering_create_info,
                .flags = ,
                .stageCount = 2,
                .pStages = shader_stage_create_infos,
                .pVertexInputState = &vertex_input_state_create_info,
                .pInputAssemblyState = input_assembly_state_create_info,

        };

        VkPipeline pipeline;
        vkCreateGraphicsPipelines(r->ldevice, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline);
}

void heph_renderer_init_compute_pipelines(HephRenderer *const r)
{       
        VkComputePipelineCreateInfo compute_pipeline_create_info = {
                .
        };


        vkCreateComputePipelines(r->ldevice, VK_NULL_HANDLE, 1, , NULL, &pipeline);
}

void heph_renderer_allocate_data_buffer(HephRenderer *const r)
{
        VkBufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = r->meshes.sb;
        HEPH_DEBUG_NOTE("Allocating " << heph_meshes_size_b(r->meshes) << " amount of bytes to data buffer.");
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT_KHR;

        VkResult res = vkCreateBuffer(r->ldevice, &info, NULL, &r->data_buffer);
        if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        {
                HEPH_ABORT("Out of GPU memory.");
        }
        else if (res != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to create vkBuffer vertex buffer.");
        }

        VkMemoryRequirements2 requirements = {};
        get_buffer_memory_requirements(r->ldevice, r->data_buffer, requirements);

        uint32_t index = get_memory_type_index(r->pdevice);

        VkMemoryAllocateInfo allocate_info = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .memoryTypeIndex = index,
                .allocationSize = r->meshes.sb;
        };

        vkAllocateMemory(r->ldevice, &allocate_info, NULL, &r->data_buffer_memory);
        if (vkBindBufferMemory(r->ldevice, r->data_buffer, r->data_buffer_memory, 0) != VK_SUCCESS)
        {
                HEPH_ABORT("Binding buffer memory failed.");
        }
}

void heph_renderer_load_mesh_data(HephRenderer *const r)
{
        void *ptr = NULL;

        if (vkMapMemory(r->ldevice, r->data_buffer_memory, 0, heph_meshes_size_b(r->meshes), 0, &ptr) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        if (ptr == NULL)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        heph_meshes_write(r->meshes, static_cast<char *>(ptr));

        vkUnmapMemory(r->ldevice, r->data_buffer_memory);
}

void heph_renderer_rebuild_swapchain(HephRenderer *const r, int width, int height)
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

#ifdef DONT_USE_THIS
static void translate_image_layout(VkCommandBuffer buffer, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
{
        VkImageMemoryBarrier2 image_barrier = {};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        /*
                TODO:
                its only a good idea to use all commands bit when youre ***NOT*** transitioning an image
                probably get rid of this function in favor of a more fine grained command bits control
        */
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        image_barrier.oldLayout = current_layout;
        image_barrier.newLayout = new_layout;

        VkImageSubresourceRange sub_range = {};
        sub_range.baseMipLevel = 0;
        sub_range.levelCount = 1;
        sub_range.baseArrayLayer = 0;
        sub_range.layerCount = 1;
        /*
                TODO
                Dont hardcode the aspect mask
        */
        sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        image_barrier.subresourceRange = sub_range;
        image_barrier.image = image;

        VkDependencyInfo dep_info = {};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;

        vkCmdPipelineBarrier2(buffer, &dep_info);
}

void *command_buffer_recording_thread_routine(void *args)
{
        if (!args)
        {
                HEPH_ABORT("Arguements passed into command buffer recording thread is NULL");
        }

        HephFrameCommandBufferRecordingInfo *recording_info = (HephFrameCommandBufferRecordingInfo *)args;

        vkResetCommandBuffer(recording_info->command_buffer, 0);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(recording_info->command_buffer, &begin_info) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to begin command buffer recording");
        }

        translate_image_layout(recording_info->command_buffer, recording_info->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkImageMemoryBarrier2 image_barrier = {};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        /*
                TODO:
                its only a good idea to use all commands bit when youre ***NOT*** transitioning an image
        */

        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;

        /*
                TODO
                i dont know if these are correct;
        */

        VkImageSubresourceRange range = {};
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        VkClearColorValue clear_color = {{255.0f, 255.0f, 255.0f, 1.0f}};

        vkCmdClearColorImage(recording_info->command_buffer, recording_info->image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

        translate_image_layout(recording_info->command_buffer, recording_info->image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        vkEndCommandBuffer(recording_info->command_buffer);

        recording_info->complete = true;

        printf("\n\n\n\n\n JUST ENDED COMMAND BUFFER RECORDING\n\n\n\n\n");

        return NULL;
}






/*
This is to setup to render. Move data onto gpu, etc. Only call this once.
The reason to use this function is so that you can instanciate (kind of) the renderer
without having to make different threads and stuff.
You can init the renderer, do some unrelated work, then call this function to fully set it up for rendering.

TODO
maybe merge this with heph_renderer_init();
*/
void heph_renderer_prepare_render(HephRenderer *const r)
{
}

#endif

static void translate_image_layout_prepare_frame_image(VkCommandBuffer buffer, VkImage image)
{
        VkImageMemoryBarrier2 barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

        /* None is sufficient, because the fences and semaphores used while aquiring the image synchronize it */
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkImageSubresourceRange sub_range = {};
        sub_range.baseMipLevel = 0;
        sub_range.levelCount = 1;
        sub_range.baseArrayLayer = 0;
        sub_range.layerCount = 1;
        sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        barrier.subresourceRange = sub_range;
        barrier.image = image;

        VkDependencyInfo dep_info = {};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(buffer, &dep_info);
}

static void translate_image_layout_frame_image_presentable(VkCommandBuffer buffer, VkImage image)
{
        VkImageMemoryBarrier2 barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        /* 
                TODO stage none is most certainly wrong here, but I dont know that is right
        */
        barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkImageSubresourceRange sub_range = {
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
        };

        barrier.subresourceRange = sub_range;
        barrier.image = image;

        VkDependencyInfo dep_info = {};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(buffer, &dep_info);
}

static void translate_image_layout_any_ONLY_USE_THIS_FOR_DEBUGGING_STUFF(VkCommandBuffer buffer, VkImage image, VkImageLayout current, VkImageLayout New)
{
        VkImageMemoryBarrier2 image_barrier = {};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        /*
                TODO:
                its only a good idea to use all commands bit when youre ***NOT*** transitioning an image
                probably get rid of this function in favor of a more fine grained command bits control
        */
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        image_barrier.oldLayout = current;
        image_barrier.newLayout = New;

        VkImageSubresourceRange sub_range = {
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
        };

        /*
                TODO
                Dont hardcode the aspect mask
        */
        sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        image_barrier.subresourceRange = sub_range;
        image_barrier.image = image;

        VkDependencyInfo dep_info = {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &image_barrier
        };

        vkCmdPipelineBarrier2(buffer, &dep_info); 
}

/* This is to actually render. Call this in a loop. */
void heph_renderer_render_frame(HephRenderer *const r, float delta_time)
{               
        uint32_t resource_index = (r->prev_resource_index + 1) * !(r->prev_resource_index + 1 == r->swapchain_nimages);

        VkFence render_complete_fence = r->frame_render_infos[resource_index].render_complete_fence;
        VkSemaphore render_complete_semaphore = r->frame_render_infos[resource_index].render_complete_semaphore;
        VkCommandBuffer command_buffer = r->frame_render_infos[resource_index].command_buffer;

        /* Make sure the fence we are going to use is not in use */
        HEPH_COND_ABORT_NE(vkWaitForFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence, VK_TRUE, HEPH_RENDERER_MAX_TIMEOUT), VK_SUCCESS);
        HEPH_COND_ABORT_NE(vkResetFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence), VK_SUCCESS);

        uint32_t image_index;
        HEPH_COND_ABORT_NE(vkAcquireNextImageKHR(r->ldevice, r->swapchain, HEPH_RENDERER_MAX_TIMEOUT, r->image_acquired_semaphore, NULL, &image_index), VK_SUCCESS);
        VkImage target_image = r->swapchain_images[image_index];

        /* Record into the command buffer */
        VkCommandBufferBeginInfo command_buffer_begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkResetCommandBuffer(command_buffer, 0);

        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);



        /*
                Invoke culling compute shader
                
        */






        translate_image_layout_any_ONLY_USE_THIS_FOR_DEBUGGING_STUFF(command_buffer, target_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        // translate_image_layout_prepare_frame_image(r->main_command_buffer, target_image);
        // translate_image_layout_frame_image_presentable(r->main_command_buffer, target_image);

        VkImageSubresourceRange range = {
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
        };

        // static bool toggle_color = 0;
        // VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, (float)toggle_color}};
        // toggle_color = !toggle_color;

        VkClearColorValue clear_color = {{0.0f, 0.0f, 255.0f, 1.0f}};

        vkCmdClearColorImage(command_buffer, target_image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

        translate_image_layout_any_ONLY_USE_THIS_FOR_DEBUGGING_STUFF(command_buffer, target_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        vkEndCommandBuffer(command_buffer);

        /* Submit the main command buffer */
        VkCommandBufferSubmitInfoKHR main_command_buffer_submit_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = command_buffer
        };

        VkSemaphoreSubmitInfo frame_render_complete_semaphore_submit_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = render_complete_semaphore,
        };

        VkSemaphoreSubmitInfo submit_info_wait_semaphores_infos[2];
        submit_info_wait_semaphores_infos[0] = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = r->image_acquired_semaphore
        }; 
        submit_info_wait_semaphores_infos[1] = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = r->frame_render_infos[r->prev_resource_index].render_complete_semaphore  
        };
        
        VkSubmitInfo2 submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &main_command_buffer_submit_info,
                .signalSemaphoreInfoCount = 1,
                .pSignalSemaphoreInfos = &frame_render_complete_semaphore_submit_info,
                .waitSemaphoreInfoCount = (uint32_t)2 - (r->prev_resource_index == UINT32_MAX),
                .pWaitSemaphoreInfos = submit_info_wait_semaphores_infos
        };

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

        r->prev_resource_index = resource_index;
}

/* Use only for debug mode. The OS is faster at cleaning up. */
void heph_renderer_destroy(HephRenderer *const r)
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
