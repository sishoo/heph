#pragma once

#include <vector>

class Renderer {
public:
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice pdevice;
    VkQueue queue;
    
    Renderer();

private:
    void init_instance();
    void init_pdevice();
    void init_device_and_queue();
    void init_swapchain();

    void destroy();
};


