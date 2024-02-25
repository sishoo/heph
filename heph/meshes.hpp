#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdio.h>

struct Meshes
{
public:
        VkDeviceSize vertex_tsb;
        VkDeviceSize index_tsb;
        std::vector<const std::string &> paths;

        static inline Meshes with_capacity(VkDeviceSize prealloc_sb);
        static inline Meshes from_path(const std::string &path);
        // static  Meshes from_dir(const std::string &path);
        static inline Meshes from_batch(const std::vector<const std::string &> &batch);

        HephResult queue_hmodl(const std::string &path);
        HephResult queue_hmodl_batch(const std::vector<const std::string> &batch_paths);
        HephResult queue_hmodl_directory(const std::string dir);

        VkDeviceSize size_b() const;
        HephResult write(char *ptr) const;

private:
        Meshes(const std::string &path);
        Meshes(VkDeviceSize prealloc_sb);
        Meshes(const std::vector<const std::string &> &batch_paths);
};