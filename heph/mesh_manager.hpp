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
    std::vector<NoInit96> vertex_normals;
    std::vector<NoInit96> vertices;
    std::vector<NoInitSize> indices;
    std::vector<NoInit32> object_strides;
    std::vector<NoInit32> objects;
    uint32_t prealloc_amount_bytes;

    void set_prealloc_amount_bytes(uint32_t size);

    HephResult load(const std::string& path);
    FileType check_file_type(const std::string& path);

};
