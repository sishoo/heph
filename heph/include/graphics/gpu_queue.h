#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


typedef struct
{
        uint32_t queue_family_index;
        VkQueue handle;
        VkQueueFlagBits flags;
} heph_gpu_queue_t; 