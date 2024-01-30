#include "renderer.hpp"
#include "heph_utils.hpp"
#include "heph_defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <cstring>
#include <ranges>

const uint32_t required_bits = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;

Renderer::Renderer()
{
    instance = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    pdevice = VK_NULL_HANDLE;
    queue = VK_NULL_HANDLE;
    
}   

Renderer::Renderer(const std::string name)
{   
    Renderer();
    init_instance(name);
    init_pdevice();
    init_device_and_queue();
    init_surface();
}

void 
Renderer::init_instance(const std::string name)
{   
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = name.c_str();
    application_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.pEngineName = "Hephaestus Engine";
    application_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);


    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;


    /* = Getting requested extensions = */
    uint32_t requested_extension_count = 0;
    std::vector<const char *> requested_extensions;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    // TODO GET THE USERS EXTENSION THING ASWELL

    requested_extensions.resize(glfw_extension_count);
    memcpy(requested_extensions.data(), glfw_extensions, glfw_extension_count * sizeof(char *));

    #ifdef HEPH_OSX
        requested_extension_count += 2;
        requested_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requested_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    #endif


    /* = Getting available extensions = */
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());


    /* = Verifying extensions are available = */
    for (size_t i = 0; i < requested_extension_count; i++)
    {
        bool extension_available = false;
        for (VkExtensionProperties properties : available_extensions)
        {
            if (!strcmp(requested_extensions[i], properties.extensionName))
            {
                extension_available = true;
                break;
            }
        }
        if (!extension_available)
        {
            HEPH_THROW_FATAL_ERROR("A requested extension is not available.");
        }
    }
    create_info.enabledExtensionCount   =   requested_extension_count;
    create_info.ppEnabledExtensionNames = requested_extensions.data();


    /* = Getting required layers = */
    enabled_layer_count = 0;
    std::vector<const char*> enabled_layers;
    #if HEPH_VALIDATE
        enabled_layer_count++;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif
    #if HEPH_MONITOR
        enabled_layer_count++;
        enabled_layers.push_back("VK_LAYER_LUNARG_monitor");
    #endif
    create_info.enabledLayerCount   =   enabled_layer_count;
    create_info.ppEnabledLayerNames = enabled_layers.data();
    enabled_layer_names = enabled_layers;


    /* = Getting create flags = */
    uint32_t create_flags = 0;
    #ifdef HEPH_OSX
        create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif
    create_info.flags = create_flags;


    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        HEPH_THROW_FATAL_ERROR("Cannot create the Vulkan instance.");
    }
}

void 
Renderer::init_pdevice()
{
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

    if (physical_device_count == 0)
        HEPH_THROW_FATAL_ERROR("There are no physical devices that support Vulkan.");

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

    auto queue_families_contain_flags = [](VkPhysicalDevice pd) {
        uint32_t property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &property_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_properties(property_count);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &property_count, queue_family_properties.data());

        for (auto props : queue_family_properties)
        {
            VkQueueFlags flags = props.queueFlags;
            if ((flags & required_bits) == required_bits)
                return true;
        }
        return false;
    };

    auto filtered_devices = physical_devices | std::ranges::views::filter(queue_families_contain_flags);
    
    if (filtered_devices.size() == 0)
        HEPH_THROW_FATAL_ERROR("Your device supports Vulkan, but is unsuitable to run Hephaestus.");

    if (filtered_devices.size() == 1) 
    {
        pdevice = filtered_devices[0];
        return;
    }

    uint32_t best_score = 0;
    VkPhysicalDevice best_device = VK_NULL_HANDLE;  
    const uint32_t device_scores[5]= { 2 , 4 , 5 , 3 , 1 };   
    for (const VkPhysicalDevice& device : filtered_devices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        uint32_t device_score = device_scores[properties.deviceType];
        if (best_score < device_score)
        {
            best_score = device_score;
            best_device = device;
        }
    }

    if (best_device == VK_NULL_HANDLE)
        HEPH_THROW_FATAL_ERROR("Your device supports Vulkan, but is unsuitable to run Hephaestus.");

    #if HEPH_VALIDATE 
        VkPhysicalDeviceProperties temp = {};
        vkGetPhysicalDeviceProperties(best_device, &temp); 
        std::cout << "Using physical device: " << temp.deviceName << '\n';
    #endif

    pdevice = best_device;
}

void 
Renderer::init_device_and_queue()
{       
    uint32_t property_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &property_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties(property_count);
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &property_count, queue_family_properties.data());

    uint32_t queue_family_index = 0;
    for (auto props : queue_family_properties)
    {   
        VkQueueFlags flags = props.queueFlags;
        if ((flags & required_bits) == required_bits)
            break;
        queue_family_index++;
    }

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    float priority = 1.0;
    queue_create_info.pQueuePriorities = &priority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};    
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledLayerCount = enabled_layer_count;
    device_create_info.ppEnabledLayerNames = enabled_layer_names.data();

    if (vkCreateDevice(pdevice, &device_create_info, nullptr, &device) != VK_SUCCESS)
        HEPH_THROW_FATAL_ERROR("Unable to create the logical device.");

    vkGetDeviceQueue(device, queue_create_info.queueFamilyIndex, 0, &queue);
}

void 
Renderer::destroy()
{   
    /* = Make sure the device work is completed before destroying stuff = */
    vkDeviceWaitIdle(device);

    vkDestroyDevice(device, nullptr);   
    vkDestroyInstance(instance, nullptr);
}