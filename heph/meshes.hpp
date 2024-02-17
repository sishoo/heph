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
    std::vector<const std::string&> paths;

    static Meshes with_capacity(size_t prealloc_sb);
    static Meshes from_path(const std::string &path);
    static Meshes from_dir(const std::string &path);
    static Meshes from_batch(const std::vector<const std::string&> &batch);

    HephResult queue_hmodl(const std::string& path);
    HephResult queue_hmodl_batch(const std::vector<const std::string>& batch_paths);
    HephResult queue_hmodl_directory(const std::string& dir);

    size_t size() const;
    HephResult write(void *const ptr) const;
};  