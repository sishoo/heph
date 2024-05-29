#pragma once

#include "../core/string.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct
{
        VkShaderModule vk_module;
        VkShaderModuleCreateInfo vk_module_create_info;
        HephConstString path;
        uint32_t *spirv_src;
        uint32_t pipeline_id;
} HephShaderModule;

void heph_shader_module_init(HephShaderModule *const m, char *const path)
{               
        
}