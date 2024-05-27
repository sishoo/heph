#pragma once

#include "types/heph_type_vertex.hpp"
#include "utils/heph_defines.hpp"
#include "types/heph_type_string.hpp"
#include "utils/heph_error.hpp"
        
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdio.h>







/*


*/

typedef struct
{
        HephString *paths;
        uint32_t nmeshes, meshes_cap;
        uint32_t *vertices_sb, *indices_sb, *normals_sb;
} HephStaticMeshes;

void heph_static_meshes_init(HephStaticMeshes *const m)
{

}

uint32_t heph_static_meshes_add_hmodl(HephStaticMeshes *const m, HephString *path)
{
        if (m->nmeshes == m->meshes_cap)
        {
                HEPH_COND_ABORT(m->paths = (HephString *)realloc(m->paths, m->meshes_cap * 2), NULL);
        }

        memcpy(&m->paths[m->nmeshes], path, sizeof(HephString));

        

        return m->nmeshes++;
}       


















// struct HephMeshes
// {
//         VkDeviceSize vertex_tsb;
//         VkDeviceSize index_tsb;
//         std::vector<const std::string> paths;
// };

// void heph_meshes_init(HephMeshes *const m);

// HephResult heph_meshes_queue_hmodl(HephMeshes *const m, const std::string &path);
// HephResult heph_meshes_queue_hmodl_batch(HephMeshes *const m, std::vector<std::string> batch_paths);
// HephResult heph_meshes_queue_hmodl_directory(HephMeshes *const m, const std::string &path);

// VkDeviceSize heph_meshes_size_b(const HephMeshes *const m);
// HephResult heph_meshes_write(const HephMeshes *const m, char *ptr);

