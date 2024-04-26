#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"
        
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdio.h>

struct HephMeshes
{
        VkDeviceSize vertex_tsb;
        VkDeviceSize index_tsb;
        std::vector<const std::string> paths;
};

void heph_meshes_init(HephMeshes *const m);

HephResult heph_meshes_queue_hmodl(HephMeshes *const m, const std::string &path);
HephResult heph_meshes_queue_hmodl_batch(HephMeshes *const m, std::vector<std::string> batch_paths);
HephResult heph_meshes_queue_hmodl_directory(HephMeshes *const m, const std::string &path);

VkDeviceSize heph_meshes_size_b(const HephMeshes *const m);
HephResult heph_meshes_write(const HephMeshes *const m, char *ptr);

