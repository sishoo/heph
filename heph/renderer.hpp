#pragma once

#include "bootstrap/VkBootstrap.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Renderer {
public:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice pdevice;
    VkDevice ldevice;
    VkQueue queue;
    VkSwapchainKHR swapchain;

    Renderer();

    void render_loop();

    void destroy();

private:
    vkb::Instance vkb_instance;
    GLFWwindow *window;
    vkb::PhysicalDevice vkb_pdevice;
    vkb::Device vkb_ldevice;
    vkb::Swapchain vkb_swapchain;

    void init_instance();
    void init_window(const std::string &name);
    void init_surface();
    void init_pdevice();
    void init_ldevice();
    void init_queue();
    void init_swapchain();
};


