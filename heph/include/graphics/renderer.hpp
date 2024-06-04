#pragma once

#include "lib/bootstrap/VkBootstrap.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <shaderc/shaderc.hpp>

#include "include/common/error.h"
#include "include/graphics/scene.h"

#define HEPH_RENDERER_HELPER_THREAD_COUNT 1
#define HEPH_RENDERER_COMMAND_BUFFER_RECORDING_THREAD_INDEX 0


/* vertex, fragment, compute culler */
#define HEPH_RENDERER_NBUILTIN_SHADER_STAGES 3

/* Maximun renderer timeout allowed */
#define HEPH_RENDERER_MAX_TIMEOUT_NS 1000000000

#define HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT 1
#define HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION 0
#define HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL 1

/* Vulkan queue must contain these bitflags */
#define HEPH_RENDERER_REQUIRED_MAIN_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)

/* Data buffer memory must adhere to these requirements */ 
#define HEPH_RENDERER_REQUIRED_GEOMETRY_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
#define HEPH_RENDERER_REQUIRED_OBJECT_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
#define HEPH_RENDERER_REQUIRED_DRAW_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)

#define VK_TRY(try)                                                                   \
        do                                                                            \
        {                                                                             \
                if ((try) != VK_SUCCESS)                                                \
                {                                                                     \
                        fprintf(stderr, "Heph Renderer VK_TRY(" #try "); failed.\n"); \
                        abort();                                                      \
                }                                                                     \
        } while (0);

typedef struct
{
        VkFence finished_fence;
        VkSemaphore finished_semaphore;
        VkSemaphoreSubmitInfo finished_semaphore_submit_info;

        VkCommandBuffer command_buffer;
} heph_frame_render_infos_t;

typedef struct
{
        uint32_t object_buffer_swap;
        heph_scene_t *scene;
        heph_camera_t *camera;
} heph_render_context_t;        

typedef struct
{       
        // heph_thread_pool_t *instance_thread_pool;

        heph_render_context_t context;

        shaderc_compiler_t shader_compiler;

        /* Core vulkan functionality */
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice ldevice;
        uint32_t queue_family_index;
        VkQueue queue;

        uint32_t nqueues, graphics_queue_index, compute_queue_index, transfer_queue_index;
        // heph_gpu_queue_t *queues;

        /* Core pipelines */
        VkPipeline graphics_pipeline, compute_pipeline;
        VkPipelineLayout graphics_pipeline_layout, compute_pipeline_layout;
        uint32_t *vertex_shader_src, *fragment_shader_src, *compute_shader_src;
        VkShaderModule vertex_shader_module, fragment_shader_module, compute_shader_module;

        /* Auxillary frame data */
        uint32_t previous_resource_index;
        VkSemaphore image_acquired_semaphore;
        heph_frame_render_infos_t *frame_render_infos;

        VkCommandPool main_command_pool;
        VkCommandPool command_buffer_recording_command_pool; /* Dedicated command pool so that you dont have to lock them */

        int window_width, window_height;
        GLFWwindow *window;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        uint32_t nswapchain_images;
        VkImage *swapchain_images;

        /* VKB */
        vkb::Instance vkb_instance;
        vkb::PhysicalDevice vkb_pdevice;
        vkb::Device vkb_ldevice;
        vkb::Swapchain vkb_swapchain;
} heph_renderer_t;
          

void heph_renderer_init(heph_renderer_t *const r, char *const window_name, int width, int height);
void heph_renderer_render_frame(heph_renderer_t *const r);
void heph_renderer_destroy(heph_renderer_t *const r);

void heph_renderer_init_instance(heph_renderer_t *const r);
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


void heph_renderer_resize(heph_renderer_t *const r, uint32_t width, uint32_t height);

void heph_renderer_rebuild_swapchain(heph_renderer_t *const r, int width, int height);



