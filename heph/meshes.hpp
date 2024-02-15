#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"

#include <vector>
#include <span>
#include <stdio.h>

struct Meshes
{   
    uint32_t num;
    size_t vertex_sb;
    size_t index_sb;
    std::vector<const std::string> paths;
    
    
    Meshes();
    Meshes(const size_t prealloc);
    Meshes(const std::string& path);
    Meshes(const const std::vector<const std::string>& batch_paths);

    HephResult queue_hmodl(const std::string& path);
    HephResult queue_hmodl_batch(const std::vector<const std::string>& batch_paths);
    HephResult queue_hmodl_directory(const std::string& dir);

    size_t size();
    HephResult write(void *const ptr);
};  