#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>

typedef struct
{
        VkBuffer handle;
        VkDeviceMemory device_memory;
        uint32_t memory_type_index;
        VkDescriptorSet descriptor_set
} heph_gpu_buffer_t;

bool heph_gpu_buffer_mmap(heph_gpu_buffer_t *const b);