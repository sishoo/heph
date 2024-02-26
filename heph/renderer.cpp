#include "renderer.hpp"
#include "utils/heph_error.hpp"
#include "log/heph_log.hpp"
#include "meshes.hpp"
#include "utils/heph_utils.hpp"

#include "bootstrap/VkBootstrap.h"
#include "bootstrap/VkBootstrap.cpp"

#include <chrono>
#include <thread>

#define REQUIRED_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)
#define REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)

#define VERTEX_SHADER_PATH "heph\\shader\\culler.comp"
#define FRAGMENT_SHADER_PATH "heph\\shader\\fragment.frag"
#define CULLER_SHADER_PATH "heph\\shader\\vertex.vert"

static void get_buffer_memory_requirements(VkDevice ldevice, VkBuffer buffer, VkMemoryRequirements2 &requirements)
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
	VkPhysicalDeviceMemoryProperties2 pdevice_properties{};
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

static void record_submit_command_buffer(VkCommandBuffer buffer, VkQueue queue)
{
	/*
		TODO:
		change the flag bits to say that it will be re recorded;
		right now we are reusing the command buffer so it does rerecord;
	*/
	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(buffer, &info);
	// TODO lambda function pointer thing probably
	vkEndCommandBuffer(buffer);
}

void heph_renderer_init(HephRenderer *const r)
{
	heph_renderer_init_instance(r);
	heph_renderer_init_window(r, "BRO WINDOW");
	heph_renderer_init_surface(r);
	heph_renderer_init_pdevice(r);
	heph_renderer_init_ldevice(r);
	heph_renderer_init_queue(r);
	heph_renderer_init_swapchain(r);
	heph_renderer_init_command_pool(r);
	heph_renderer_allocate_command_buffers(r);
}

void heph_renderer_init_instance(HephRenderer *const r)
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
	r->vkb_instance = res.value();
	r->instance = r->vkb_instance.instance;
}

void heph_renderer_init_window(HephRenderer *const r, const std::string &name)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	if ((r->window = glfwCreateWindow(800, 600, name.c_str(), NULL, NULL)) == NULL)
	{
		HEPH_ABORT("Unable to create window.");
	}
}

void heph_renderer_init_surface(HephRenderer *const r)
{
	if (glfwCreateWindowSurface(r->instance, r->window, NULL, &r->surface) != VK_SUCCESS)
	{
		HEPH_ABORT("Unable to create surface.");
	}
}

void heph_renderer_init_pdevice(HephRenderer *const r)
{
	printf("DONT DELETE THIS PRINTF UNTIL YOU FIX THE DAMN PICK FIRST DEVICE UNCONDITIONALLY.");
	vkb::PhysicalDeviceSelector selector(r->vkb_instance);
	auto res = selector
		       .set_surface(r->surface)
		       .select_first_device_unconditionally()
		       .select();
	if (!res)
	{
		HEPH_ABORT("Unable to find a physical device to use: " << res.error().message());
	}
	r->vkb_pdevice = res.value();
	r->pdevice = r->vkb_pdevice.physical_device;
}

void heph_renderer_init_ldevice(HephRenderer *const r)
{
	vkb::DeviceBuilder builder{r->vkb_pdevice};
	auto res = builder.build();
	if (!res)
	{
		HEPH_ABORT("Unable to create logical device: " << res.error().message());
	}
	r->vkb_ldevice = res.value();
	r->ldevice = r->vkb_ldevice.device;
}

void heph_renderer_init_queue(HephRenderer *const r)
{
	uint32_t i = 0;
	for (VkQueueFamilyProperties props : r->vkb_pdevice.get_queue_families())
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
	r->queue_family_index = i;
	vkGetDeviceQueue(r->ldevice, i, 0, &(r->queue));
}

void heph_renderer_init_swapchain(HephRenderer *const r)
{
	vkb::SwapchainBuilder builder{r->vkb_ldevice};
	auto res = builder.build();
	if (!res)
	{
		HEPH_ABORT("Unable to create swapchain: " << res.error().message());
	}
	r->vkb_swapchain = res.value();
	r->swapchain = r->vkb_swapchain.swapchain;
}

void heph_renderer_init_command_pool(HephRenderer *const r)
{
	/*
		TODO
		we are using the command reset bit but int he future when we write commands every frame we need to change that
	*/
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = r->queue_family_index;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(r->ldevice, &info, NULL, &r->command_pool) != VK_SUCCESS)
	{
		HEPH_ABORT("Failed to allocate command pool.");
	}
}

void heph_renderer_allocate_command_buffers(HephRenderer *const r)
{
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandBufferCount = 1;
	info.commandPool = r->command_pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(r->ldevice, &info, &r->command_buffer) != VK_SUCCESS)
	{
		HEPH_ABORT("Failed to allocate command buffers");
	}
}

void heph_renderer_allocate_data_buffer(HephRenderer *const r)
{
	VkBufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = r->meshes.size_b();
	DEBUG_NOTE("Allocating " << r->meshes.size_b() << " amount of bytes to data buffer.");
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.usage = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT_KHR;

	VkResult res = vkCreateBuffer(r->ldevice, &info, NULL, &r->data_buffer);
	if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
	{
		HEPH_ABORT("Out of GPU memory.");
	}
	else if (res != VK_SUCCESS)
	{
		HEPH_ABORT("Failed to create vkBuffer vertex buffer.");
	}

	VkMemoryRequirements2 requirements{};
	get_buffer_memory_requirements(r->ldevice, r->data_buffer, requirements);

	uint32_t index = get_memory_type_index(r->pdevice);

	VkMemoryAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.memoryTypeIndex = index;
	allocate_info.allocationSize = r->meshes.size_b();

	vkAllocateMemory(r->ldevice, &allocate_info, NULL, &r->data_buffer_memory);
	if (vkBindBufferMemory(r->ldevice, r->data_buffer, r->data_buffer_memory, 0) != VK_SUCCESS)
		HEPH_ABORT("Binding buffer memory failed.");
}

void heph_renderer_run(HephRenderer *const r)
{
	bool drawing = true;
	while (!glfwWindowShouldClose(r->window))
	{
		glfwPollEvents();

		// handle events
		// if minimize tab / alt tab stop drawing

		if (!drawing)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		// render()
	}
}

void heph_renderer_load_mesh_data(HephRenderer *const r)
{
	heph_renderer_allocate_data_buffer(r);
	void *ptr = NULL;

	if (vkMapMemory(r->ldevice, r->data_buffer_memory, 0, r->meshes.size_b(), 0, &ptr) != VK_SUCCESS)
	{
		HEPH_ABORT("Failed to map memory.");
	}

	if (ptr == NULL)
	{
		HEPH_ABORT("Failed to map memory.");
	}

	r->meshes.write(static_cast<char *>(ptr));

	vkUnmapMemory(r->ldevice, r->data_buffer_memory);
}

void heph_renderer_rebuild_swapchain(HephRenderer *const r, int width, int height)
{
	vkDeviceWaitIdle(r->ldevice);

	vkb::destroy_swapchain(r->vkb_swapchain);
	r->swapchain = VK_NULL_HANDLE;

	/*
		TODO
		finish the swap chain recreateion
		paused on it becuase its a bit of jumping the gun
	*/
}

void heph_renderer_destroy(HephRenderer *const r)
{
	vkDeviceWaitIdle(r->ldevice);

	glfwDestroyWindow(r->window);
	glfwTerminate();

	/* Do not change ordering */
	vkFreeMemory(r->ldevice, r->data_buffer_memory, NULL);
	vkDestroyBuffer(r->ldevice, r->data_buffer, NULL);
	vkb::destroy_swapchain(r->vkb_swapchain);
	vkb::destroy_surface(r->vkb_instance, r->surface);
	vkb::destroy_device(r->vkb_ldevice);
	vkb::destroy_instance(r->vkb_instance);
}
