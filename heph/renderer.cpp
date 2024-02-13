#include "renderer.hpp"
#include "utils/heph_error.hpp"
#include "utils/heph_defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <string.h>

Renderer::Renderer()
{
    init_instance();
    init_pdevice();
    init_device_and_queue();
    init_surface();
}

static void 
init_application_info(VkApplicationInfo &info)
{
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Hephaestus Renderer";
    info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    info.pEngineName = "Hephaestus Engine";
    info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
}

static std::vector<const char *const> 
get_required_extensions()
{
#ifdef HEPH_OSX
    #define extensions_size 2
    std::vector<const char *> extensions{VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
#else
    #define extensions_size 0
    std::vector<const char *> extensions;
#endif
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    extensions.resize(glfw_extension_count + extensions_size);
    memcpy(extensions.end(), glfw_extensions, glfw_extension_count * sizeof(char *));
    return extensions;
}

static bool 
verify_extensions_available(const std::vector<std::string>& required, const std::vector<VkExtensionProperties>& available)
{
    for (auto e : required)
    {
        bool is_available = false;
        for (auto a : available)
        {
            is_available |= !strcmp(e.c_str(), a.extensionName);
        }
        if (!is_available)
        {
            HEPH_PRINT_ERROR("Extension not found: '" << e << "'.");
            return false;
        }
    }
}

static std::vector<VkExtensionProperties>
get_available_extensions()
{
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions;
    available_extensions.resize(available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());
    return available_extensions;
}

void 
Renderer::init_instance()
{
    VkApplicationInfo application_info;
    init_application_info(application_info);

    std::vector<const char *const> extensions = get_required_extensions();
    std::vector<VkExtensionProperties> available_extensions = get_available_extensions();

    if (!verify_extensions_available(extensions, available_extensions))
        HEPH_THROW_FATAL_ERROR("Not all required extensions available.");

#if HEPH_MONITOR
    const char *const layers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
#elif HEPH_VALIDATE
    const char *const layers[] = {"VK_LAYER_KHRONOS_validation"};
#endif

#ifdef HEPH_OSX
    const uint32_t flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    const uint32_t flags = 0;
#endif

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = app_info;
    create_info.enabledExtensionCount = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount = sizeof(layers) / sizeof(char *);
    create_info.ppEnabledLayerNames = layers;
    create_info.flags = flags;

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
        HEPH_THROW_FATAL_ERROR("Cannot create the Vulkan instance.");
}



void 
Renderer::init_pdevice()
{
    uint32_t pdevice_count = 0;
    vkEnumeratePhysicalDevices(instance, &pdevice_count, nullptr);

    if (pdevice_count == 0)
        HEPH_THROW_FATAL_ERROR("There are no physical devices that support Vulkan.");

    std::vector<VkPhysicalDevice> pdevices(pdevice_count);
    vkEnumeratePhysicalDevices(instance, &pdevice_count, pdevices.data());

    auto queue_families_contain_flags = [](VkPhysicalDevice pd)
    {
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
    const uint32_t device_scores[5] = {2, 4, 5, 3, 1};
    for (const VkPhysicalDevice &device : filtered_devices)
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

void Renderer::init_device_and_queue()
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

void Renderer::destroy()
{
    /* = Make sure the device work is completed before destroying stuff = */
    vkDeviceWaitIdle(device);

    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}