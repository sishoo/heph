#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"

#include <vector>
#include <span>
#include <stdio.h>

struct Meshes
{
    std::vector<Vertex> vertex_buffer;
    std::vector<uint32_t> index_buffer;
    std::vector<Vec3> normal_buffer;

    HephResult load_hmodl(const std::string& path);
    HephResult load_hmodl_batch(const std::string& dir);

    HephResult load_obj(const std::string& path);
    HephResult load_obj_batch(const std::string& dir);
};