


#include "renderer.hpp"
#include "heph_utils.hpp"
#include "defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <string>


void Renderer::init_instance(std::string name)
{   
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = name.c_str();
    application_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.pEngineName = "Hephaestus Engine";
    application_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    
    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    uint32_t enabled_layer_count = 0;
    std::vector<const char*> enabled_layers;
    #if HEPH_VALIDATE
        enabled_layer_count++;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif
    #if HEPH_MONITOR
        enabled_layer_count++;
    #endif


    
    

    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        HEPH_THROW_ERROR_UNRECOVERABLE("Cannot create vulkan instance");
    }
}






