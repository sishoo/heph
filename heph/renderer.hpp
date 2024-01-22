#pragma once

#include <vector>

#include "mesh_manager.hpp"

class Renderer {
public:
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice pdevice;
    VkQueue queue;
    MeshManager meshes;
    
    Renderer(const std::string name);

    

private:
    Renderer();

    uint32_t enabled_layer_count;
    std::vector<const char*> enabled_layer_names;

    void init_instance(std::string name);
    void init_pdevice();
    void init_device_and_queue();
    void init_swapchain();

    void destroy();
};