#pragma once

#include <vector>
#include <stdint.h>
#include <string>

#include "heph_defines.hpp"
#include "heph_utils.hpp"

#include "types/no_init.hpp"
#include "types/vertex.hpp"
#include "types/vec.hpp"

typedef enum FileType {
    Unsupported,
    Hmodl,
    Obj
};

typedef struct LoadingInfo {
    uint32_t vertex_size;
    uint32_t index_size;
    uint32_t vertex_normals_size;
} LoadingInfo;

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


    template<typename T>
    HEPH_RESULT load_into_buffer(const std::string& path, std::vector<T>& buffer);


    

    HEPH_RESULT calc_insert_vertex_normals(size_t vertices_start, size_t indices_start);

};
