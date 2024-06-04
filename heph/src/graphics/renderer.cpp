#include "include/common/error.h"
#include "include/graphics/renderer.hpp"
#include "include/core/log.hpp"
// #include "include/core/thread_pool.h"
#include "include/core/string.h"
#include "include/graphics/gpu_queue.h"
#include "include/core/memory.h"
#include "include/utils/file_helper.h"

#include "lib/bootstrap/VkBootstrap.cpp"

#include <shaderc/shaderc.hpp>
#include <stdint.h>

void heph_renderer_init_instance(heph_renderer_t *const r)
{
        vkb::InstanceBuilder builder;
        auto res = builder
                .set_app_name("Hephaestus Renderer")
                .set_engine_name("Hephaestus Engine")
                .require_api_version(1, 3, 0)
                .use_default_debug_messenger()
              //.set_debug_callback(debug_callback)
                .request_validation_layers()
                .build();
        HEPH_NASSERT(res.vk_result(), VK_SUCCESS);
        r->vkb_instance = res.value();
        r->instance = r->vkb_instance.instance;
}

void heph_renderer_init_window(heph_renderer_t *const r, char *const name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        HEPH_ASSERT((r->window = glfwCreateWindow(r->window_width, r->window_height, name, NULL, NULL)), NULL);
}

void heph_renderer_init_surface(heph_renderer_t *const r)
{
        HEPH_NASSERT(glfwCreateWindowSurface(r->instance, r->window, NULL, &r->surface), VK_SUCCESS);
}

void heph_renderer_init_pdevice(heph_renderer_t *const r)
{
        #warning DONT DELETE THIS UNTIL YOU FIX THE DAMN PICK FIRST DEVICE UNCONDITIONALLY!!!
        vkb::PhysicalDeviceSelector selector(r->vkb_instance);
        auto res = selector
                       .set_surface(r->surface)
                       .select_first_device_unconditionally()
                       .select();
        HEPH_NASSERT(res.vk_result(), VK_SUCCESS);
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
        HEPH_NASSERT(res.vk_result(), VK_SUCCESS);
        r->vkb_ldevice = res.value();
        r->ldevice = r->vkb_ldevice.device;
}

void heph_renderer_init_queues(heph_renderer_t *const r)
{
        /* First pass to find a queue with everything */
        #warning subject to change
        uint32_t required = HEPH_RENDERER_REQUIRED_MAIN_QUEUE_FAMILY_BITFLAGS, i = 0, nqueues = 0;
        for (VkQueueFamilyProperties props : r->vkb_pdevice.get_queue_families())
        {
                VkQueueFlags flags = props.queueFlags;
                if ((flags & required) == required)
                {
                        goto found_all;
                }
                i++;
                nqueues++;
        }

        found_all:
        r->nqueues = 1;
        vkGetDeviceQueue(r->ldevice, i, 0, &r->queue);

#ifdef DONT_USE_THIS
        while (required)
        {

        }
        for (VkQueueFamilyProperties props : r->vkb_pdevice.get_queue_families())
        {
                
        }





        /* Second pass to find multiple queues that meet requirements */
        i = 0;
        while (required)
        {
                uint32_t flag = flags & required;
                switch (flag)
                {
                        case 0:
                        {
                                HEPH_ABORT("Unable to find suitable queues.");
                        }
                        case VK_QUEUE_GRAPHICS_BIT:    
                        {
                                nqueues++;
                                heph_gpu_queue_t graphics_queue = {     
                                        .queue_family_index = i
                                };
                                vkGetDeviceQueue(r->ldevice, i, 0, &graphics_queue.handle);
                                break;
                        } 
                        case VK_QUEUE_COMPUTE_BIT: 
                        {
                                nqueues++;
                                heph_gpu_queue_t compute_queue = {
                                        .queue_family_index = i      
                                };
                                vkGetDeviceQueue(r->ldevice, i, 0, &compute_queue.handle);
                                break;
                        }
                        case VK_QUEUE_TRANSFER_BIT:
                        {
                                nqueues++;
                                heph_gpu_queue_t transfer_queue = {
                                        .queue_family_index = i
                                };
                                vkGetDeviceQueue(r->ldevice, i, 0, &transfer_queue.handle);
                                break;
                        }
                }
                required &= !flags;
                i++;
        }

        found_all:


#endif

}

void heph_renderer_init_swapchain(heph_renderer_t *const r)
{
        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder
                       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .build();
        HEPH_NASSERT(res.vk_result(), VK_SUCCESS);
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;
}

void heph_renderer_aquire_swapchain_images(heph_renderer_t *const r)
{
        HEPH_NASSERT(vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->nswapchain_images, NULL), VK_SUCCESS);

        r->swapchain_images = (VkImage *)calloc(sizeof(VkImage), r->nswapchain_images);

        HEPH_NASSERT(vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->nswapchain_images, r->swapchain_images), VK_SUCCESS);

#if HEPH_VALIDATE
        for (uint32_t i = 0; i < r->nswapchain_images; i++)
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
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        };

        HEPH_NASSERT(vkCreateCommandPool(r->ldevice, &main_command_pool_create_info, NULL, &r->main_command_pool), VK_SUCCESS);

        /* Recording thread command pool */
        VkCommandPoolCreateInfo command_buffer_recording_command_pool = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = r->queue_family_index
        };

        VkCommandPool recording_thread_command_pool = VK_NULL_HANDLE;
        HEPH_NASSERT(vkCreateCommandPool(r->ldevice, &command_buffer_recording_command_pool, NULL, &r->command_buffer_recording_command_pool), VK_SUCCESS);
}

void heph_renderer_init_sync_structures(heph_renderer_t *const r)
{
        /* Image acquired */
        VkSemaphoreCreateInfo image_acquired_semaphore_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        HEPH_NASSERT(vkCreateSemaphore(r->ldevice, &image_acquired_semaphore_info, NULL, &r->image_acquired_semaphore), VK_SUCCESS);
}

void heph_renderer_init_frame_render_infos(heph_renderer_t *const r)
{
        r->previous_resource_index = UINT32_MAX;
        r->frame_render_infos = (heph_frame_render_infos_t *)HCALLOC(r->nswapchain_images, sizeof(heph_frame_render_infos_t));

        VkFenceCreateInfo finished_fence_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        VkSemaphoreCreateInfo finished_semaphore_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = r->command_buffer_recording_command_pool,
                .commandBufferCount = 1,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
        };

        for (uint32_t i = 0; i < r->nswapchain_images; i++)
        {
                HEPH_NASSERT(vkCreateFence(r->ldevice, &finished_fence_info, NULL, &r->frame_render_infos[i].finished_fence), VK_SUCCESS);
                HEPH_NASSERT(vkCreateSemaphore(r->ldevice, &finished_semaphore_info, NULL, &r->frame_render_infos[i].finished_semaphore), VK_SUCCESS);
                HEPH_NASSERT(vkAllocateCommandBuffers(r->ldevice, &command_buffer_allocate_info, &r->frame_render_infos[i].command_buffer), VK_SUCCESS);
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
                shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->size_bytes, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_NASSERT(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->size_bytes = shaderc_result_get_length(result);
}

void heph_renderer_preprocess_fragment_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
                shaderc_compile_into_preprocessed_text(compiler, src->ptr, src->size_bytes, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_NASSERT(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->size_bytes = shaderc_result_get_length(result);
}

void heph_renderer_compile_vertex_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
                shaderc_compile_into_spv(compiler, src->ptr, src->size_bytes, shaderc_vertex_shader, "vertex_shader", "main", compile_options);

        HEPH_NASSERT(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->size_bytes = shaderc_result_get_length(result);
}

void heph_renderer_compile_fragment_shader(shaderc_compiler_t compiler, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
                shaderc_compile_into_spv(compiler, src->ptr, src->size_bytes, shaderc_fragment_shader, "fragment_shader", "main", compile_options);

        HEPH_NASSERT(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->size_bytes = shaderc_result_get_length(result);
}

void heph_renderer_init_shader_compiler(heph_renderer_t *const r)
{
        r->shader_compiler = shaderc_compiler_initialize();
        HEPH_ASSERT(r->shader_compiler, NULL);
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
        HEPH_NASSERT(heph_file_read_to_string(&vertex_shader_src, "shader/vertex.vert"), true);
        heph_renderer_preprocess_vertex_shader(r->shader_compiler, &vertex_shader_src);
        heph_renderer_compile_vertex_shader(r->shader_compiler, &vertex_shader_src);

        VkShaderModuleCreateInfo vertex_shader_create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = vertex_shader_src.size_bytes,
                .pCode = (uint32_t *)vertex_shader_src.ptr}
        ;

        /* Fragment shader */
        heph_string_t fragment_shader_src = {};
        HEPH_NASSERT(heph_file_read_to_string(&fragment_shader_src, "shader/fragment.frag"), true);
        heph_renderer_preprocess_fragment_shader(r->shader_compiler, &fragment_shader_src);
        heph_renderer_compile_fragment_shader(r->shader_compiler, &fragment_shader_src);

        VkShaderModuleCreateInfo fragment_shader_create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = fragment_shader_src.size_bytes,
                .pCode = (uint32_t *)fragment_shader_src.ptr
        };
}

void heph_renderer_init_graphics_pipelines(heph_renderer_t *const r)
{
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
        uint32_t ntotal_shaders = ;
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
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE
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
                .viewportCount = HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor
        };

        /* Rasterization State */
        #warning this is janky come backe and properly do this (EX: cull mode)
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_FRONT_BIT,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0
        };

        /* Multisample state */
        // VkPipelineMultisampleStateCreateInfo multisample_state = {
        //         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        //         .flags = 
        // };

        /* Final grahpics pipeline */
        VkGraphicsPipelineCreateInfo pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &pipeline_rendering_create_info,
                .stageCount = 2,
                .pStages = shader_stage_create_infos,
                .pVertexInputState = &vertex_input_state_create_info,
                .pInputAssemblyState = &input_assembly_state_create_info,

        };

        VkPipeline pipeline;
        HEPH_NASSERT(vkCreateGraphicsPipelines(r->ldevice, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline), VK_SUCCESS);
}

void heph_renderer_preprocess_compute_shader(heph_renderer_t *const r, heph_string_t *const src)
{
}

void heph_renderer_compile_compute_shader(heph_renderer_t *const r, heph_string_t *const src)
{
        shaderc_compile_options_t compile_options = shaderc_compile_options_initialize();

        shaderc_compilation_result_t result =
            shaderc_compile_into_spv(r->shader_compiler, src->ptr, src->size_bytes, shaderc_compute_shader, "compute_shader", "main", compile_options);

        HEPH_NASSERT(shaderc_result_get_compilation_status(result), shaderc_compilation_status_success);

        HFREE(src->ptr);
        src->ptr = (char *)shaderc_result_get_bytes(result);
        src->size_bytes = shaderc_result_get_length(result);
}

void heph_renderer_init_compute_pipelines(heph_renderer_t *const r)
{
        heph_string_t compute_shader_src = {};
        HEPH_NASSERT(heph_file_read_to_string(&compute_shader_src, "shader/compute.comp"), true);
        heph_renderer_preprocess_compute_shader(r, &compute_shader_src);
        heph_renderer_compile_compute_shader(r, &compute_shader_src);

        /* Shader Module */
        VkShaderModuleCreateInfo compute_shader_module_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = compute_shader_src.size_bytes,
            .pCode = (uint32_t *)compute_shader_sr.ptr
        };

        VkShaderModule compute_shader_module = {};
        VK_TRY(vkCreateShaderModule(r->ldevice, &compute_shader_module_create_info, NULL, &compute_shader_module));

        /* Shader stage */
        VkPipelineShaderStageCreateInfo compute_shader_stage_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = compute_shader_module,
            .pName = "main"
        };

        /* Push constant range */
        VkPushConstantRange push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .offset = 0,
                .size = sizeof(uint32_t) + sizeof(float[6]) + sizeof(float[16])
        };

        /* Descriptor sets */
        VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[];

        /* Object buffer descriptor set */
        descriptor_set_layout_bindings[0] = {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        };

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT,
                .bindingCount = 1,
                .pBinding = &descriptor_set_layout_bindings[0]
        };

        VkDescriptorSetLayout descriptor_set_layout = {};
        VK_TRY(vkCreateDescriptorSetLayout(r->ldevice, &descriptor_set_layout_create_info, NULL, &descriptor_set_layout));

        /* Pipeline layout */
        VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info = {
                .sType = VK_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 3,
                .pSetLayouts = descriptor_set_layouts,
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &pipeline_layout_push_constant_range
        };

        VK_TRY(vkCreatePipelineLayout(r->ldevice, &pipeline_layout_create_info, NULL, &r->compute_pipeline_layout));

        /* Create compute pipeline */
        VkComputePipelineCreateInfo compute_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                .stage = compute_shader_stage_create_info,
                .layout =,
                .basePipelineHandle =,
                .basePipelineIndex = 
        };

        VK_TRY(vkCreateComputePipelines(r->ldevice, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &r->compute_pipeline));
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

        HEPH_NASSERT(res.vk_result(), VK_SUCCESS);

        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;

        heph_renderer_aquire_swapchain_images(r);
}

void heph_renderer_handle_window_resize(heph_renderer_t *const r, int width, int height)
{
        r->window_width = width;
        r->window_height = height;
        heph_renderer_rebuild_swapchain(r, width, height);
        heph_renderer_recalculate_projection_matrix(r);
}

void heph_renderer_render(heph_renderer_t *const r)
{
        for (uint32_t s = 0; s < r->nscenes; s++)
        {
                r->context.scene = &r->scenes[s];

                for (uint32_t c = 0; c < r->context.scene.ncamera; c++)
                {
                        r->context.camera = &r->context.scene->cameras[c];

                        heph_renderer_render_frame(r);
                }
        }
}

/* Renders a single frame */
void heph_renderer_render_frame(heph_renderer_t *const r)
{
        heph_scene_t *scene = r->context.scene;
        heph_camera_t *camera = r->context.camera;

        uint32_t resource_index = r->previous_resource_index;
        if (++resource_index == r->nswapchain_images)
        {
                resource_index = 0;
        }

        heph_frame_render_infos_t *frame_infos = &r->frame_render_infos[resource_index];

        /* Make sure the fence we are going to use is not in use */
        HEPH_ASSERT(vkWaitForFences(r->ldevice, 1, &frame_infos->complete_fence, VK_TRUE, HEPH_RENDERER_MAX_TIMEOUT), VK_SUCCESS);
        HEPH_ASSERT(vkResetFences(r->ldevice, 1, &frame_infos->complete_fence), VK_SUCCESS);

        /* Get target image */
        uint32_t image_index;
        HEPH_ASSERT(vkAcquireNextImageKHR(r->ldevice, r->swapchain, HEPH_RENDERER_MAX_TIMEOUT, r->image_acquired_semaphore, NULL, &image_index), VK_SUCCESS);
        VkImage target_image = r->swapchain_images[image_index];

        /* Pepare the frame command buffer for recording */
        VkCommandBufferBeginInfo command_buffer_begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        VK_TRY(vkResetCommandBuffer(frame_infos->command_buffer, 0));
        VK_TRY(vkBeginCommandBuffer(frame_infos->command_buffer, &command_buffer_begin_info));

        /* Get ready to compute cull */
        /* Dispatch culling compute shader */
        vkCmdBindPipeline(frame_infos->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, r->compute_pipeline);

        /* PushConstant the frustum and object buffer swap boolean */
        camera->__push_constant_padding = scene->object_buffer_swap;
        vkCmdPushConstants(
                frame_infos->command_buffer,
                r->compute_pipeline_layout,
                VK_SHADER_STAGE_COMPUTE_BIT,
                0,
                sizeof(heph_camera_t),
                camera->__push_constant_padding
        );

        vkCmdBindDescriptorSets(frame_infos->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, r->compute_pipeline_layout, 0, 3, scene->scene_buffers_descriptor_sets, 0, NULL);
        vkCmdDispatch(frame_infos->command_buffer, ceil(nobjects / 16), 1, 1);

        /* Sync access to draw buffer */
        VkBufferMemoryBarrier2 draw_buffer_memory_barrier = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                .srcQueueFamilyIndex = r->queue_family_index,
                .dstQueueFamilyIndex =,
                .buffer = scene->draw_buffer,
                .offset = 0,
                .size = VK_WHOLE_SIZE
        };

        /* Translate target image: UNDEFINED -> COLOR_ATTACHMENT */
        VkImageSubresourceRange target_image_subresource_range = {
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
        };

        VkImageMemoryBarrier2 target_image_memory_barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_NONE, // None is sufficient, aquiring the image is synced
                .dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image = target_image,
                .subresourceRange = target_image_subresource_range
        };

        /* Barrier for image transition UNDEFINED -> COLOR_ATTACHMENT and compute culler */
        VkDependencyInfo barriers_dependency_info = {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .bufferMemoryBarrierCount = 1,
                .pBufferMemoryBarriers = &draw_buffer_memory_barrier,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &target_image_memory_barrier
        };

        vkCmdPipelineBarrier2(frame_infos->command_buffer, &barriers_dependency_info);

        /* Bind required 'graphics' resources */
        vkCmdBindVertexBuffers(frame_infos->command_buffer, 0, 1, &scene->geometry_buffer.handle, (uint32_t[]){0});
        vkCmdBindIndexBuffer(frame_infos->command_buffer, scene->geometry_buffer.handle, scene->meshes_size_bytes, VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(frame_infos->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

        /* PushConstant the view / projection matrices */
        vkCmdPushConstants(
                frame_infos->command_buffer,
                r->graphics_pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT,
                sizeof(camera->view_matrix) + sizeof(uint32_t),
                sizeof(scene->projection_matrix),
                scene->projection_matrix;
        );

        vkCmdBindDescriptorSets(
                frame_infos->command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                r->graphics_pipeline_layout,
                0,
                1,
                &scene->texture_buffer.descriptor_set,
                0,
                NULL
        );

        for (uint32_t i = 0; i < scene->nobjects; i++)
        {
                heph_object_t *object = &(heph_object_t *)scene->object_buffer.mapped_ptr[i];
                if (!object->is_visible)
                {
                        continue;
                }
                vkCmdPushConstants(
                    frame_infos->command_buffer,
                    r->graphics_pipeline_layout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    sizeof(camera->view_matrix) + sizeof(scene->projection_matrix) + sizeof(uint32_t),
                    sizeof(uint32_t),
                    &i
                );
        }

        vkCmdDrawIndexedIndirect(frame_infos->command_buffer, scene->draw_buffer, 0, , sizeof(VkDrawIndexedIndirectCommand));

        /* Translate target image: COLOR_ATTACHMENT -> PRESENTABLE */
        VkImageMemoryBarrier2 target_image_memory_barrier2 = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_NONE, // None is sufficient, acquiring the image is synced
                .dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image = target_image,
                .subresourceRange = target_image_subresource_range
        };

        /* Barrier for image transition COLOR_ATTACHMENT -> PRESENTABLE */
        // TODO bad name
        VkDependencyInfo presentation_dependency_info = {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &target_image_memory_barrier2
        };

        vkCmdPipelineBarrier2(frame_infos->command_buffer, &presentation_dependency_info);

        VK_TRY(vkEndCommandBuffer(frame_infos->command_buffer));

        /* Submit the main command buffer */
        VkCommandBufferSubmitInfoKHR main_command_buffer_submit_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = frame_infos->command_buffer
        };

        VkSemaphoreSubmitInfo finished_semaphore_submit_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = frame_infos->finished_semaphore,
                .flags = 
        };      

        VkSemaphoreSubmitInfo submit_info_wait_semaphores[2] = {};
        /* Wait on image acquired semaphore */
        submit_info_wait_semaphores[0] = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = r->image_acquired_semaphore
        };
        /* Wait on previous frame done semaphore */
        submit_info_wait_semaphores[1] = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = frame_infos->finished_semaphore
        };

        VkSubmitInfo2 submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &main_command_buffer_submit_info,
                .signalSemaphoreInfoCount = 1,
                .pSignalSemaphoreInfos = &frame_infos->finished_semaphore_submit_info,
                .waitSemaphoreInfoCount = (uint32_t)2 - (r->previous_resource_index == UINT32_MAX),
                .pWaitSemaphoreInfos = submit_info_wait_semaphores
        };

        VK_TRY(vkQueueSubmit2(r->queue, 1, &submit_info, frame_infos->finished_fence));

        /* Present the frame to the screen */
        VkPresentInfoKHR present_info = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .swapchainCount = 1,
                .pSwapchains = &r->swapchain,
                .waitSemaphoreCount = 1,
                .pImageIndices = &image_index,
                .pWaitSemaphores = &frame_infos->finished_semaphore,
        };

        VK_TRY(vkQueuePresentKHR(r->queue, &present_info));

#ifdef USE_THIS_LATER_BRUH
        heph_scene_t *scene = r->current_scene;
        heph_camera_t *camera = r->current_camera;

        uint32_t resource_index = (r->resource_index + 1) * !(r->resource_index + 1 == r->swapchain_nimages);

        VkFence render_complete_fence = r->frame_render_infos[resource_index].render_complete_fence;
        VkSemaphore render_complete_semaphore = r->frame_render_infos[resource_index].render_complete_semaphore;
        VkCommandBuffer frame_command_buffer = r->frame_render_infos[resource_index].command_buffer;

        /* Make sure the fence we are going to use is not in use */
        HEPH_ASSERT(vkWaitForFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence, VK_TRUE, HEPH_RENDERER_MAX_TIMEOUT), VK_SUCCESS);
        HEPH_ASSERT(vkResetFences(r->ldevice, 1, &r->frame_render_infos[resource_index].render_complete_fence), VK_SUCCESS);

        uint32_t image_index;
        HEPH_ASSERT(vkAcquireNextImageKHR(r->ldevice, r->swapchain, HEPH_RENDERER_MAX_TIMEOUT, r->image_acquired_semaphore, NULL, &image_index), VK_SUCCESS);
        VkImage target_image = r->swapchain_images[image_index];

        /* Pepare the frame command buffer for recording */
        VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

        vkResetCommandBuffer(command_buffer, 0);
        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

        /* Get ready to compute cull */
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
            &r->current_camera->__push_constant_padding);

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
            .size = r->nobjects};

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
        vkCmdBindVertexBuffers(frame_command_buffer, 0, 1, scene->geometry_buffer(uint32_t[]){0});
        vkCmdBindIndexBuffer(frame_command_buffer, scene->geometry_buffer, scene->vertices_size_bytes, VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(frame_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

        /* Gather required textures */

        for (uint32_t i = 0; i < scene->nobjects; i++)
        {
                heph_object_t object = scene->objects[i];
                if (object.is_visible)
                {
                        /* Send the texture */
                        scene->mapped_object_buffer[i].
                }
        }

        /* Get ready to draw */

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

        HEPH_ASSERT(vkQueueSubmit2(r->queue, 1, &submit_info, r->frame_render_infos[resource_index].render_complete_fence), VK_SUCCESS);

        /* Present the frame to the screen */
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &r->swapchain,
            .waitSemaphoreCount = 1,
            .pImageIndices = &image_index,
            .pWaitSemaphores = &render_complete_semaphore,
        };

        HEPH_ASSERT(vkQueuePresentKHR(r->queue, &present_info), VK_SUCCESS);
#endif

        r->previous_resource_index = resource_index;
        r->object_buffer_swap = !r->object_buffer_swap;
}

void heph_renderer_init(heph_renderer_t *const r, char *const window_name, int width, int height)
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
        HFREE(r->vertex_shader_src);
        HFREE(r->fragment_shader_src);
        HFREE(r->compute_shader_src);

        for (uint32_t i = 0; i < r->nswapchain_images; i++)
        {
                vkDestroyFence(r->ldevice, r->frame_render_infos[i].finished_fence, NULL);
                vkDestroySemaphore(r->ldevice, r->frame_render_infos[i].finished_semaphore, NULL);
        }
        HFREE(r->frame_render_infos);

        /* Do not change ordering */
        for (uint32_t i = 0; i < r->nswapchain_images; i++)
        {
                vkDestroyImage(r->ldevice, r->swapchain_images[i], NULL);
        }
        HFREE(r->swapchain_images);



        vkDestroyCommandPool(r->ldevice, r->main_command_pool, NULL);



        /* Destroy main buffers */
        uint32_t nmain_buffers = sizeof(r->scene->main_buffers) / sizeof(r->scene->main_buffers[0]);
        for (uint32_t i = 0; i < nmain_buffers; i++)
        {
                vkFreeMemory(r->ldevice, r->scene->main_buffers[i].device_memory, NULL);
                vkDestroyBuffer(r->ldevice, r->scene->main_buffers[i].handle, NULL);
        }


        vkb::destroy_swapchain(r->vkb_swapchain);
        vkb::destroy_surface(r->vkb_instance, r->surface);
        vkb::destroy_device(r->vkb_ldevice);
        vkb::destroy_instance(r->vkb_instance);
}
