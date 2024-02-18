#pragma once

#include "bootstrap/VkBootstrap.h"
#include "utils/heph_error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

struct Renderer {
public:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice pdevice;
    VkDevice ldevice;
    VkQueue queue;
    VkSwapchainKHR swapchain;

    VkBuffer data_buffer;
    VkDeviceMemory data_buffer_memory;
    VkDeviceSize vertex_offset;
    VkDeviceSize index_offset;
    VkDeviceSize normal_offset;

    Renderer(const Meshes &meshes);

    void load_mesh_data();
    void render_loop();

    void destroy();

private:
    vkb::Instance vkb_instance;
    GLFWwindow *window;
    vkb::PhysicalDevice vkb_pdevice;
    vkb::Device vkb_ldevice;
    vkb::Swapchain vkb_swapchain;
    const Meshes &meshes;

    void init_instance();
    void init_window(const std::string &name);
    void init_surface();
    void init_pdevice();
    void init_ldevice();
    void init_queue();
    void init_swapchain();

    HephResult allocate_data_buffer();
};


