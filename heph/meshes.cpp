#include "meshes.hpp"

#include "utils/heph_defines.hpp"
#include "utils/heph_error.hpp"
#include "utils/heph_file_helper.hpp"

#ifdef HEPH_OSX
#include <sys/xattr.h>
#endif

// Meshes::Meshes(size_t prealloc_sb) : vertex_tsb(0), index_tsb(0), paths(prealloc_sb) {}

// Meshes::Meshes(const std::string &path) : vertex_tsb(0), index_tsb(0) { paths.push_back(path); }

// Meshes::Meshes(const std::vector<const std::string &> &batch_paths) : vertex_tsb(0), index_tsb(0) { paths = batch_paths; }

// inline Meshes
// Meshes::with_capacity(size_t prealloc_sb)
// {
//     return Meshes(prealloc_sb);
// }

// inline Meshes
// Meshes::from_path(const std::string &path)
// {
//     return Meshes(path);
// }

// // Meshes
// // Meshes::from_dir(const std::string &dir)
// // {

// // }

// inline Meshes
// Meshes::from_batch(const std::vector<const std::string &> &batch_paths)
// {
//     return Meshes(batch_paths);
// }

Meshes::Meshes(const std::string path)
{
    vertex_tsb = 0;
    index_tsb = 0;
    queue_hmodl(path);
}

HephResult
Meshes::queue_hmodl(const std::string path)
{
    if (!file_exists(path))
    {
        HEPH_PRINT_ERROR("Unable to queue file for loading (file does not exist) path`: " << path << ".");
        return HephResult::Failure;
    }

    char val[HMODL_XATTR_VALUE_SIZE];
    if (getxattr(path.c_str(), HMODL_XATTR_NAME, val, HMODL_XATTR_VALUE_SIZE, 0, 0) == -1)
    {
        HEPH_PRINT_ERROR("Failed to get xattr of: " << path);
        return HephResult::Failure;
    }
    vertex_tsb += ((uint32_t *)val)[0];
    index_tsb += ((uint32_t *)val)[1];
    paths.push_back(path);

    return HephResult::Failure;
}

HephResult
Meshes::queue_hmodl_batch(const std::vector<const std::string> &batch_paths)
{
    uint32_t result = static_cast<uint32_t>(HephResult::Success);
    paths.reserve(batch_paths.size());
    for (const std::string &path : batch_paths)
    {   
        result &= static_cast<uint32_t>(queue_hmodl(path));
    }
    return static_cast<HephResult>(result);
}

HephResult
Meshes::queue_hmodl_directory(const std::string dir_path)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dir_path.c_str())) == NULL)
    {
        HEPH_PRINT_ERROR("Failed to open directory.");
        return HephResult::Failure;
    }
    while((ent = readdir(dir)) != NULL)
    {
        queue_hmodl(ent->d_name);
    }
    closedir(dir);
    return HephResult::Success;
}

HephResult
Meshes::write(char *ptr) const
{
    char *vertex_ptr = ptr;
    char *index_ptr = ptr + vertex_tsb;
    char *normal_ptr = index_ptr + index_tsb;
    for (const std::string &path : paths)
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
        if (getxattr(path.c_str(), HMODL_XATTR_NAME, val, HMODL_XATTR_VALUE_SIZE, 0, 0) == -1)
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


size_t
Meshes::size_b() const
{
    return 2 * vertex_tsb + index_tsb;
}