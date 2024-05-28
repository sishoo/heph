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
        heph_instance_t *parent_instance;        
        heph_thread_pool_t *instance_thread_pool;



        /* Core vulkan functionality */
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice ldevice;
        uint32_t queue_family_index;
        VkQueue queue;


        // TODO this is a one camera system, for multi camera, you need different view matrices
        heph_camera_t camera;
        float projection_matrix[16];


        /* Engine Buffers */
        uint32_t object_buffer_swap; // interpereted as a bool
        VkBuffer geometry_buffer, object_buffer, draw_buffer;
        VkDeviceMemory object_buffer_memory, geometry_buffer_memory, draw_buffer_memory;
        const uint32_t geometry_buffer_memory_type_index, object_buffer_memory_type_index, draw_buffer_memory_type_index;




        uint32_t npipelines;
        uint32_t *vertex_shader_src, *fragment_shader_src, *compute_shader_src;
        VkShaderModule vertex_shader_module, fragment_shader_module, compute_shader_module;




        VkPipeline graphics_pipeline, compute_pipeline;
        VkPipelineLayout graphics_pipeline_layout, compute_pipeline_layout;









        
        




        /* Frame presentation */
        GLFWwindow *window;
        int window_width, window_height;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        uint32_t swapchain_nimages;
        VkImage *swapchain_images;



        /* Auxillary frame data */
        uint32_t resource_index;
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
} heph_renderer_t;

void heph_renderer_init(heph_renderer_t *const *const r, char *const window_name, int width, int height);
void heph_renderer_setup(heph_renderer_t *const *const r);
void heph_renderer_run(heph_renderer_t *const *const r);
void heph_renderer_render(heph_renderer_t *const r);
void heph_renderer_destroy(heph_renderer_t *const r);
 
void heph_renderer_init_instance(heph_renderer_t *const r);
void heph_renderer_init_window(heph_renderer_t *const r, char *const name);
void heph_renderer_init_surface(heph_renderer_t *const r);
void heph_renderer_init_pdevice(heph_renderer_t *const r);
void heph_renderer_init_ldevice(heph_renderer_t *const r); 
void heph_renderer_init_queue(heph_renderer_t *const r);
void heph_renderer_init_swapchain(heph_renderer_t *const r);
void heph_renderer_aquire_swapchain_images(heph_renderer_t *const r);
void heph_renderer_init_command_pools(heph_renderer_t *const r);
void heph_renderer_allocate_main_command_buffer(heph_renderer_t *const r);
void heph_renderer_init_sync_structures(heph_renderer_t *const r);
void heph_renderer_init_frame_render_infos(heph_renderer_t *const r);

void heph_renderer_rebuild_swapchain(heph_renderer_t *const r, int width, int height);

