#pragma once

#include "bootstrap/VkBootstrap.h"
#include "utils/heph_error.hpp"
#include "meshes.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

struct HephRenderer
{
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice ldevice;

        uint32_t queue_family_index;
        VkQueue queue;
        
        GLFWwindow *window;
        int window_width, window_height;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;

        VkBuffer data_buffer;
        VkDeviceMemory data_buffer_memory;
        VkDeviceSize vertex_offset;
        VkDeviceSize index_offset;
        VkDeviceSize normal_offset;

        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;

        vkb::Instance vkb_instance;
        vkb::PhysicalDevice vkb_pdevice;
        vkb::Device vkb_ldevice;
        vkb::Swapchain vkb_swapchain;

        const Meshes &meshes;
};

void heph_renderer_init(HephRenderer *const r);
void heph_renderer_run(HephRenderer *const r);
void heph_renderer_destroy(HephRenderer *const r);
 
void heph_renderer_init_instance(HephRenderer *const r);
void heph_renderer_init_window(HephRenderer *const r, const std::string &name);
void heph_renderer_init_surface(HephRenderer *const r);
void heph_renderer_init_pdevice(HephRenderer *const r);
void heph_renderer_init_ldevice(HephRenderer *const r);
void heph_renderer_init_queue(HephRenderer *const r);
void heph_renderer_init_swapchain(HephRenderer *const r);
void heph_renderer_init_command_pool(HephRenderer *const r);
void heph_renderer_allocate_command_buffers(HephRenderer *const r);

void heph_renderer_rebuild_swapchain(HephRenderer *const r, int width, int height);