#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"

#include <vector>
#include <span>
#include <stdio.h>

struct Meshes
{   
public:
    size_t vertex_tsb;
    size_t index_tsb;
    std::vector<const std::string&> paths;

    static inline Meshes with_capacity(size_t prealloc_sb);
    static inline Meshes from_path(const std::string &path);
    static inline Meshes from_dir(const std::string &path);
    static inline Meshes from_batch(const std::vector<const std::string &> &batch);

    HephResult queue_hmodl(const std::string &path);
    HephResult queue_hmodl_batch(const std::vector<const std::string &> &batch_paths);
    HephResult queue_hmodl_directory(const std::string &dir);

    size_t size() const;
    HephResult write(void *ptr) const;

private:
    Meshes(size_t prealloc_sb);
    Meshes(const std::string &path);
    Meshes(const std::vector<const std::string &> &batch_paths);

    
};  