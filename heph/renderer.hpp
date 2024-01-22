#pragma once




#include <vector>

class Renderer {
public:
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice pdevice;

    Renderer() = delete;
    Renderer(const std::string name);

    MeshManager meshes;

private:
    void init_instance(std::string name);
    void init_pdevice();
    void init_device();
    void init_swapchain();

    void destroy();
};