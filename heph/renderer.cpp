#include "renderer.hpp"
#include "utils/heph_error.hpp"

#include "bootstrap/VkBootstrap.h"
#include "bootstrap/VkBootstrap.cpp"

#include <vector>

#define REQUIRED_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)

Renderer::Renderer()
{
    init_instance();
    init_window("BRO WINDOW");
    init_surface();
    init_pdevice();
    init_ldevice();
    init_queue();
    init_swapchain();
}

void
Renderer::init_instance()
{
    vkb::InstanceBuilder builder;
    auto res = builder
        .set_app_name("Hephaestus Renderer")
        .set_engine_name("Hephaestus Engine")
        .require_api_version(1, 3, 0)
        .use_default_debug_messenger()
        .request_validation_layers()
        .build();
    if (!res)
    {
        HEPH_THROW_FATAL_ERROR("Unable to create Vulkan Instance: " << res.error().message());
    }
    vkb_instance = res.value();
    instance = vkb_instance.instance;
}

void
Renderer::init_window(const std::string &name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if ((window = glfwCreateWindow(800, 600, name.c_str(), NULL, NULL)) == NULL)
    {
        HEPH_THROW_FATAL_ERROR("Unable to create window.");
    }
}

void
Renderer::init_surface()
{
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
    {
        HEPH_THROW_FATAL_ERROR("Unable to create surface.");
    }
}

void
Renderer::init_pdevice()
{
    vkb::PhysicalDeviceSelector selector(vkb_instance);
    auto res = selector
        .set_surface(surface)
        .select_first_device_unconditionally()
        .select();
    if (!res)
    {
        HEPH_THROW_FATAL_ERROR("Unable to find a physical device to use: " << res.error().message());
    }
    vkb_pdevice = res.value();
    pdevice = vkb_pdevice.physical_device;
}

void
Renderer::init_ldevice()
{
    vkb::DeviceBuilder builder{vkb_pdevice};
    auto res = builder.build();
    if (!res)
    {
        HEPH_THROW_FATAL_ERROR("Unable to create logical device: " << res.error().message());
    }
    vkb_ldevice = res.value();
    ldevice = vkb_ldevice.device;
}

void
Renderer::init_queue()
{   
    uint32_t i = 0;
    for (VkQueueFamilyProperties props : vkb_pdevice.get_queue_families())
    {
        VkQueueFlags flags = props.queueFlags;
        if ((flags & REQUIRED_QUEUE_FAMILY_BITFLAGS) == REQUIRED_QUEUE_FAMILY_BITFLAGS)
        {
            goto found;
        }
        i++;
    }
    HEPH_THROW_FATAL_ERROR("Unable to find suitable queue for Hephaestus to use");
found:
    vkGetDeviceQueue(ldevice, i, 0, &queue);
}

void
Renderer::init_swapchain()
{
    vkb::SwapchainBuilder builder{vkb_ldevice};
    auto res = builder.build();
    if (!res)
    {
        HEPH_THROW_FATAL_ERROR("Unable to create swapchain: " << res.error().message());
    }
    vkb_swapchain = res.value();
    swapchain = vkb_swapchain.swapchain;
}

void 
Renderer::render_loop()
{
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void
Renderer::destroy()
{
    vkDeviceWaitIdle(ldevice);

    glfwDestroyWindow(window);
    glfwTerminate();
   
    vkb::destroy_device(vkb_ldevice);
    vkb::destroy_surface(vkb_instance, surface);
    vkb::destroy_swapchain(vkb_swapchain);

    vkb::destroy_instance(vkb_instance);
}



