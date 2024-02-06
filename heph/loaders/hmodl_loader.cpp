#include "hmodl_loader.hpp"

#include "../utils/heph_utils.hpp"
#include "../utils/heph_defines.hpp"

HephResult
HMODLloader::load(const std::string& path)
{
    

    uint32_t fd = 0;
    void *mapped_ptr;
    if (!(mapped_ptr = map_file(path, &fd)))
    {
        HEPH_PRINT_ERROR("Failed to map the file: " << path << " to memory.");
        return HephResult::Failure;
    }

    HMODLloadingInfo *info = (HMODLloadingInfo *)mapped_ptr;
    
    mapped_ptr += HMODL_HEADER_SIZE_BYTES;

    Vec3* vertices_ptr = (Vec3 *) &*vertices.end();
    vertices.resize(vertices.size() + info->vertex_size);
    memcpy(vertices_ptr, mapped_ptr, info->vertex_size);

    mapped_ptr += info->vertex_size;

    size_t *indices_ptr = (size_t *) &*indices.end();
    indices.resize(indices.size() + info->index_size);
    memcpy(indices_ptr, mapped_ptr, info->index_size);

    mapped_ptr += info->index_size;

    Vec3 *vertex_normals_ptr = (Vec3 *) &*vertex_normals.end();
    vertex_normals.resize(vertex_normals.size() + info->vertex_normals_size);
    memcpy(vertex_normals_ptr, mapped_ptr, info->vertex_normals_size);






    return HephResult::Success;
}



