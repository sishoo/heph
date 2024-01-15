#pragma once

#include <vector>
#include <stdint.h>
#include <string>


#include "heph_defines.hpp"
#include "vec.hpp"
#include "loader.hpp"


class MeshManager {
public:
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> object_strides;
    std::vector<Mat4> model_matrices;
    
    HEPH_RESULT load_mesh(const std::string& path);

    MeshManager() : mesh_loader(Loader());
private:
    Loader mesh_loader;
    uint32_t vertices_cursor;
    uint32_t indices_cursor;
};
