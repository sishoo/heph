#include "renderer.hpp"
#include "utils/heph_error.hpp"

#include "bootstrap/VkBootstrap.h"
#include "bootstrap/VkBootstrap.cpp"
#include "meshes.hpp"
#include "utils/heph_utils.hpp"

#define REQUIRED_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)
#define REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)

#define VERTEX_SHADER_PATH "heph\\shader\\culler.comp"
#define FRAGMENT_SHADER_PATH "heph\\shader\\fragment.frag"
#define CULLER_SHADER_PATH "heph\\shader\\vertex.vert"

Renderer::Renderer(const Meshes &meshes) : meshes(meshes)
{
        init_instance();
        init_window("BRO WINDOW");
        init_surface();
        init_pdevice();
        init_ldevice();
        init_queue();
        init_swapchain();
}

void Renderer::init_instance()
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
                HEPH_ABORT("Unable to create Vulkan Instance: " << res.error().message());
        }
        vkb_instance = res.value();
        instance = vkb_instance.instance;
}

void Renderer::init_window(const std::string &name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if ((window = glfwCreateWindow(800, 600, name.c_str(), NULL, NULL)) == NULL)
        {
                HEPH_ABORT("Unable to create window.");
        }
}

void Renderer::init_surface()
{
        if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
        {
                HEPH_ABORT("Unable to create surface.");
        }
}

void Renderer::init_pdevice()
{
        vkb::PhysicalDeviceSelector selector(vkb_instance);
        auto res = selector
                       .set_surface(surface)
                       .select_first_device_unconditionally()
                       .select();
        if (!res)
        {
                HEPH_ABORT("Unable to find a physical device to use: " << res.error().message());
        }
        vkb_pdevice = res.value();
        pdevice = vkb_pdevice.physical_device;
}

void Renderer::init_ldevice()
{
        vkb::DeviceBuilder builder{vkb_pdevice};
        auto res = builder.build();
        if (!res)
        {
                HEPH_ABORT("Unable to create logical device: " << res.error().message());
        }
        vkb_ldevice = res.value();
        ldevice = vkb_ldevice.device;
}

void Renderer::init_queue()
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
        HEPH_ABORT("Unable to find suitable queue for Hephaestus to use");
found:
        vkGetDeviceQueue(ldevice, i, 0, &queue);
}

void Renderer::init_swapchain()
{
        vkb::SwapchainBuilder builder{vkb_ldevice};
        auto res = builder.build();
        if (!res)
        {
                HEPH_ABORT("Unable to create swapchain: " << res.error().message());
        }
        vkb_swapchain = res.value();
        swapchain = vkb_swapchain.swapchain;
}

static void get_memory_requirements(VkDevice ldevice, VkBuffer buffer, VkMemoryRequirements2 &requirements)
{
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        VkBufferMemoryRequirementsInfo2 info;
        info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        info.pNext = NULL;
        info.buffer = buffer;
        vkGetBufferMemoryRequirements2(ldevice, &info, &requirements);
}

static uint32_t get_memory_type_index(VkPhysicalDevice pdevice)
{
        VkPhysicalDeviceMemoryProperties2 pdevice_properties;
        memset(&pdevice_properties, 0, sizeof(VkPhysicalDeviceMemoryProperties2));
        pdevice_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(pdevice, &pdevice_properties);

        uint32_t i = 0;
        for (VkMemoryType type : pdevice_properties.memoryProperties.memoryTypes)
        {
                if ((type.propertyFlags & REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS) == REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS)
                {
                        return i;
                }
                i++;
        }
        HEPH_ABORT("Cannot find adequate memory type.");
}

HephResult Renderer::allocate_data_buffer()
{
        VkBufferCreateInfo info;
        memset(&info, 0, sizeof(VkBufferCreateInfo));
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = meshes.size_b();
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT_KHR;

        VkResult res = vkCreateBuffer(ldevice, &info, NULL, &data_buffer);
        if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        {
                HEPH_ABORT("Out of GPU memory.");
        }
        else if (res != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to create vkBuffer vertex buffer.");
        }

        VkMemoryRequirements2 requirements;
        memset(&requirements, 0, sizeof(VkMemoryRequirements2));
        get_memory_requirements(ldevice, data_buffer, requirements);

        uint32_t index = get_memory_type_index(pdevice);

        VkMemoryAllocateInfo allocate_info;
        memset(&allocate_info, 0, sizeof(VkMemoryAllocateInfo));
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.memoryTypeIndex = index;
        allocate_info.allocationSize = meshes.size_b();

        vkAllocateMemory(ldevice, &allocate_info, NULL, &data_buffer_memory);
        if (vkBindBufferMemory(ldevice, data_buffer, data_buffer_memory, 0) != VK_SUCCESS)
        {
                HEPH_ABORT("Binding buffer memory failed.");
        }

        return HephResult::Success;
}

void Renderer::load_mesh_data()
{
        allocate_data_buffer();
        void *ptr = NULL;

        VkMemoryMapInfoKHR map_info;
        memset(&map_info, 0, sizeof(VkMemoryMapInfoKHR));
        map_info.sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR;
        map_info.memory = data_buffer_memory;
        map_info.size = meshes.size_b();

        if (vkMapMemory2KHR(ldevice, &map_info, &ptr) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        if (ptr == NULL)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        meshes.write(static_cast<char *>(ptr));

        VkMemoryUnmapInfoKHR unmap_info;
        memset(&unmap_info, 0, sizeof(VkMemoryUnmapInfoKHR));
        unmap_info.sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO_KHR;
        unmap_info.memory = map_info.memory;

        if (vkUnmapMemory2KHR(ldevice, &unmap_info) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to unmap memory.");
        }
}

static uint32_t *compile_vertex_shader(size_t *size)
{
}

void Renderer::render_loop()
{
        size_t vertex_shader_size = 0;
        uint32_t *vertex_shader_src = compile_vertex_shader(&vertex_shader_size);

        size_t  fragment_shader_size = 0;
        uint32_t *fragment_shader_src = compile_fragment_shader();

        size_t culler_shader_size = 0;
        uint32_t *culler_shader_src = compile_culler_shader();

        if (!vertex_shader_size || !fragment_shader_size || !culler_shader_size)
        {
                HEPH_ABORT("Failed to compile shaders.")
        }

        


        while (!glfwWindowShouldClose(window))
        {
        }
}

Renderer::~Renderer()
{
        vkDeviceWaitIdle(ldevice);

        glfwDestroyWindow(window);
        glfwTerminate();

        /* Do not change ordering */
        vkFreeMemory(ldevice, data_buffer_memory, NULL);
        vkDestroyBuffer(ldevice, data_buffer, NULL);
        vkb::destroy_swapchain(vkb_swapchain);
        vkb::destroy_surface(vkb_instance, surface);
        vkb::destroy_device(vkb_ldevice);
        vkb::destroy_instance(vkb_instance);
}
