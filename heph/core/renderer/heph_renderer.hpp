#pragma once

#include "heph_renderer_defines.hpp"

#include "bootstrap/VkBootstrap.h"
#include "utils/heph_error.hpp"
#include "heph_thread_pool.hpp"
#include "../meshes.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>

#define HEPH_RENDERER_HELPER_THREAD_COUNT 1
#define HEPH_RENDERER_COMMAND_BUFFER_RECORDING_THREAD_INDEX 0


typedef struct 
{       
        VkImage image;
        VkCommandBuffer command_buffer;
        bool complete;
} HephFrameCommandBufferRecordingInfo;

typedef struct
{
        VkFence render_complete_fence;
        VkCommandBuffer command_buffer;
        VkSemaphore render_complete_semaphore;
} HephFrameRenderInfos;

typedef struct 
{       
        shaderc_compiler_t shader_compiler;

        uint32_t prev_resource_index;
        HephThreadPool *heph_application_thread_pool;

        /* Core vulkan functionality */
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice ldevice;
        uint32_t queue_family_index;
        VkQueue queue;





        /* Engine Buffers */
        VkBuffer geometry_buffer, object_buffer, draw_buffer;
        VkDeviceMemory object_buffer_memory, geometry_buffer_memory, draw_buffer_memory;
        const uint32_t geometry_buffer_memory_type_index, object_buffer_memory_type_index, draw_buffer_memory_type_index;





        uint32_t npipelines, nadditional_shader_modules;
        uint32_t *vertex_shader_src, *fragment_shader_src, *compute_shader_src;
        VkShaderModule vertex_shader_module, fragment_shader_module, compute_shader_module;
        VkShaderModule *additional_shader_modules;
        VkPipeline *pipelines;
        VkPipeline compute_pipeline;










        
        




        /* Frame presentation */
        GLFWwindow *window;
        int window_width, window_height;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        uint32_t swapchain_nimages;
        VkImage *swapchain_images;



        /* Auxillary frame data */
        VkSemaphore image_acquired_semaphore;
        HephFrameRenderInfos *frame_render_infos;

        VkCommandPool main_command_pool;
        VkCommandPool command_buffer_recording_command_pool; /* Dedicated command pool so that you dont have to lock them */



        // HephStaticMesh *static_meshes;




        /* VKB */
        vkb::Instance vkb_instance;
        vkb::PhysicalDevice vkb_pdevice;
        vkb::Device vkb_ldevice;
        vkb::Swapchain vkb_swapchain;
} HephRenderer;

void heph_renderer_init(HephRenderer *const r, char *const window_name, int width, int height);
void heph_renderer_setup(HephRenderer *const r);
void heph_renderer_run(HephRenderer *const r);
void heph_renderer_render(HephRenderer *const r);
void heph_renderer_destroy(HephRenderer *const r);
 
void heph_renderer_init_instance(HephRenderer *const r);
void heph_renderer_init_window(HephRenderer *const r, char *const name);
void heph_renderer_init_surface(HephRenderer *const r);
void heph_renderer_init_pdevice(HephRenderer *const r);
void heph_renderer_init_ldevice(HephRenderer *const r); 
void heph_renderer_init_queue(HephRenderer *const r);
void heph_renderer_init_swapchain(HephRenderer *const r);
void heph_renderer_aquire_swapchain_images(HephRenderer *const r);
void heph_renderer_init_command_pools(HephRenderer *const r);
void heph_renderer_allocate_main_command_buffer(HephRenderer *const r);
void heph_renderer_init_sync_structures(HephRenderer *const r);
void heph_renderer_init_frame_render_infos(HephRenderer *const r);

void heph_renderer_rebuild_swapchain(HephRenderer *const r, int width, int height);

