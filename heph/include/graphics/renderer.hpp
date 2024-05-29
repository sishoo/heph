#pragma once


#include "lib/bootstrap/VkBootstrap.h"

#include "../common/error.hpp"
#include "../core/thread_pool.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <shaderc/shaderc.hpp>

#define HEPH_RENDERER_HELPER_THREAD_COUNT 1
#define HEPH_RENDERER_COMMAND_BUFFER_RECORDING_THREAD_INDEX 0


/* Vulkan queue must contain these bitflags */
#define HEPH_RENDERER_REQUIRED_MAIN_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)

/* Data buffer memory must adhere to these requirements */ 
#define HEPH_RENDERER_REQUIRED_GEOMETRY_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
#define HEPH_RENDERER_REQUIRED_OBJECT_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
#define HEPH_RENDERER_REQUIRED_DRAW_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)



/* vertex, fragment, compute culler */
#define HEPH_RENDERER_NBUILTIN_SHADER_STAGES 3

/* Maximun renderer timeout allowed */
#define HEPH_RENDERER_MAX_TIMEOUT_NS 1000000000


#define HEPH_RENDERER_PIPELINE_VIEWPORT_COUNT 1
#define HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_POSITION 0
#define HEPH_RENDERER_VERTEX_INPUT_ATTR_DESC_BINDING_NORMAL 1

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
} heph_frame_render_infos_t;

typedef struct
{
        heph_instance_t *parent_instance;        
        // heph_thread_pool_t *instance_thread_pool;
        heph_renderer_backend_t *backend;

        heph_scene_t *scene;

        /* Core vulkan functionality */
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice ldevice;
        uint32_t queue_family_index;
        VkQueue queue;

        /* Core pipelines */
        VkPipeline graphics_pipeline, compute_pipeline;
        VkPipelineLayout graphics_pipeline_layout, compute_pipeline_layout;
        uint32_t *vertex_shader_src, *fragment_shader_src, *compute_shader_src;
        VkShaderModule vertex_shader_module, fragment_shader_module, compute_shader_module;

        /* Auxillary frame data */
        uint32_t resource_index;
        VkSemaphore image_acquired_semaphore;
        heph_frame_render_infos_t *frame_render_infos;

        VkCommandPool main_command_pool;
        VkCommandPool command_buffer_recording_command_pool; /* Dedicated command pool so that you dont have to lock them */

        GLFWwindow *window;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        uint32_t swapchain_nimages;
        VkImage *swapchain_images;

        /* VKB */
        vkb::Instance vkb_instance;
        vkb::PhysicalDevice vkb_pdevice;
        vkb::Device vkb_ldevice;
        vkb::Swapchain vkb_swapchain;
} heph_renderer_t;

void heph_renderer_init(heph_renderer_t *const r, uint32_t nviewports, heph_viewport_t *const v);
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

