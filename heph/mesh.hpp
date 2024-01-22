#pragma once

#include <vector>

#include "vec.hpp"
#include "mat.hpp"

typedef struct Mesh {
    uint32_t vertices_start;
    uint32_t vertices_num;
    uint32_t indices_start;
    uint32_t indices_num;
    uint32_t mat_index;

    Mesh() : vertex_start(0), vertex_num(0), index_start(0), index_num(0) {}

} Mesh;

