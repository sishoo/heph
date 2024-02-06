#pragma once

#include <vector>
#include <stdint.h>
#include <string>

#include "heph_defines.hpp"
#include "heph_utils.hpp"

#include "types/no_init.hpp"
#include "types/vertex.hpp"
#include "types/vec.hpp"

enum FileType {
    Unsupported,
    Hmodl,
    Obj
};

struct LoadingInfo {
    uint32_t vertex_size;
    uint32_t index_size;
    uint32_t vertex_normals_size;
};

#define HMODL_HEADER_SIZE_BYTES sizeof(LoadingInfo)

class MeshManager {
public:

    
    std::vector<NoInit<Vertex>> vertices;
    std::vector<NoInit<uint32_t>> indices;
    std::vector<NoInit<Vec3>> vertex_normals;

    std::vector<NoInit<uint32_t>> object_strides;
    std::vector<NoInit<uint32_t>> objects;


    HephResult map_file(char **ptr_to_ptr, const std::string& path);
    HephResult load(const std::string& path);   


    FileType check_file_type(const std::string& path);
};
