#include "meshes.hpp"

#include "utils/heph_defines.hpp"
#include "utils/heph_error.hpp"
#include "utils/heph_file_helper.hpp"
#include "hmodl/hmodl.hpp"

void heph_meshes_init(HephMeshes *const m)
{
}

HephResult heph_meshes_queue_hmodl(HephMeshes *const m, const std::string &path)
{
        if (!heph_file_exists(path))
        {
                HEPH_PRINT_ERROR("Unable to queue file for loading (file does not exist) path`: " << path << ".");
                return HephResult::Failure;
        }

        char val[9];
        if (!heph_get_hmodl_xattr(path, val))
        {
                HEPH_PRINT_ERROR("Failed to get xattr of: " << path);
                return HephResult::Failure;
        }
        m->vertex_tsb += ((uint32_t *)val)[0];
        m->index_tsb += ((uint32_t *)val)[1];

        m->paths.push_back(path);

        return HephResult::Success;
}

HephResult heph_meshes_queue_hmodl_batch(HephMeshes *const m, std::vector<const std::string> batch_paths)
{
        HephResult result = HephResult::Success;
        m->paths.reserve(batch_paths.size());
        for (const std::string &path : batch_paths)
        {
                result = result & heph_meshes_queue_hmodl(m, path);
        }
        return result;
}

HephResult heph_meshes_queue_hmodl_directory(HephMeshes *const m, const std::string &path)
{
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(path.c_str())) == NULL)
        {
                HEPH_PRINT_ERROR("Failed to open directory.");
                return HephResult::Failure;
        }
        while ((ent = readdir(dir)) != NULL)
        {
                heph_meshes_queue_hmodl(m, ent->d_name);
        }
        closedir(dir);
        return HephResult::Success;
}

HephResult heph_meshes_write(const HephMeshes *const m, char *ptr)
{
        char *vertex_ptr = ptr;
        char *index_ptr = ptr + m->vertex_tsb;
        char *normal_ptr = index_ptr + m->index_tsb;
        for (const std::string &path : m->paths)
        {
                int fd = open(path.c_str(), O_RDONLY);
                if (fd == -1)
                {
                        HEPH_ABORT("Cannot write " << path << " to memory.");
                }
                struct stat status;
                if (fstat(fd, &status) == -1)
                {
                        HEPH_ABORT("Cannot write " << path << " to memory.");
                }
                char val[HMODL_XATTR_VALUE_SIZE];
                if (!heph_get_hmodl_xattr(path, val))
                {
                        HEPH_ABORT("Failed to get xattr of" << path);
                }
                size_t vertex_sb = ((uint32_t *)val)[0];
                size_t index_sb = ((uint32_t *)val)[1];

                if (pread(fd, vertex_ptr, vertex_sb, 0) == -1)
                {
                        HEPH_ABORT("Cannot write model vertex data to GPU.");
                }
                vertex_ptr += vertex_sb;

                if (pread(fd, index_ptr, index_sb, vertex_sb) == -1)
                {
                        HEPH_ABORT("Cannot write model index data to GPU.");
                }
                index_ptr += index_sb;

                if (pread(fd, index_ptr, vertex_sb, vertex_sb + index_sb) == -1)
                {
                        HEPH_ABORT("Failed to write model normals to GPU.");
                }
                normal_ptr += vertex_sb;
        }

        return HephResult::Success;
}

VkDeviceSize heph_meshes_size_b(const HephMeshes *const m)
{
        return 2 * m->vertex_tsb + m->index_tsb;
}