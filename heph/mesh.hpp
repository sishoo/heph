#pragma once

#include <vector>

#include "vec.hpp"
#include "mat.hpp"

typedef struct Mesh {
    std::vector<Point3> vertex_buffer;
    std::vector<size_t> index_buffer;
    std::vector<size_t> material_index;
    Mat4 model_matrix;
    AABB bounds;

    Mesh() = delete;

} Mesh;

