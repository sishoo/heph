#pragma once

#include <vector>
#include <stdint.h>
#include <string>


#include "heph_defines.hpp"
#include "vec.hpp"
#include "loader.hpp"
#include "vertex.hpp"


typedef enum FileType {
    Unsupported,
    Hmodl,
    Obj
};

class MeshManager {
public:
    std::vector<Vec3> vertex_normals;
    std::vector<Vertex> vertices;
    std::vector<size_t> indices;
    std::vector<uint32_t> object_strides;
    std::vector<uint32_t> objects;

    HephResult load(const std::string& path);
    FileType check_file_type(const std::string& path);


    template<typename T>
    HEPH_RESULT load_into_buffer(const std::string& path, std::vector<T>& buffer);


    

    HEPH_RESULT calc_insert_vertex_normals(size_t vertices_start, size_t indices_start);

};
