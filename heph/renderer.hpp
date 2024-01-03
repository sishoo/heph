#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class Renderer {
public:
    VkInstance instance;
    VkDevice device;


    Renderer();





private:
    void init_instance(std::string name);



};