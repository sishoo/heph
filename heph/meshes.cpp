#include "meshes.hpp"

#include "utils/heph_error.hpp"
#include "utils/heph_file_helper.hpp"


HephResult
Meshes::queue_hmodl(const std::string& path)
{
    if (!file_exists(path))
    {
        HEPH_PRINT_ERROR("Unable to queue file for loading (file does not exist): " << path << ".");
        return HephResult::Failure;
    }
    paths.push_back(path);
}

HephResult
Meshes::queue_hmodl_batch(const std::vector<const std::string>& batch_paths)
{
    HephResult result = HephResult::Success;
    paths.reserve(batch_paths.size());
    for (const std::string path : batch_paths)
    {
        if (!file_exists(path))
        {
            result = HephResult::Failure;
            HEPH_PRINT_ERROR("Unable to load file: " << path << " of HMODL batch.");
            continue;
        }
        paths.push_back(path);
    }
    return result;
}

HephResult
Meshes::queue_hmodl_directory(const std::string& dir)
{
    std::vector<std::string> entries = enumerate_directory_entries(dir);
    
}












/*
HephResult
Meshes::load_hmodl(const std::string& path)
{
    size_t size = 0;
    void *file = open_file(path, &size, "r");
    if (!file)
    {
        HEPH_PRINT_ERROR("Faild to open file: " << path << ".");
        return HephResult::Failure;
    }
    
    HMODLHeader *info = (HMODLHeader *)file;
    
    void *vertex_end = &*vertex_buffer.end();
    void *index_end  = &*index_buffer.end();
    void *normal_end = &*normal_buffer.end();

    vertex_buffer.resize(vertex_buffer.size() + info->vertex_sb / sizeof(Vertex));
    index_buffer.resize(index_buffer.size() + info->index_sb / sizeof(uint32_t));
    normal_buffer.resize(normal_buffer.size() + info->normal_sb / sizeof(Vec3));

    memcpy(vertex_end, file + info->vertex_offset, info->vertex_sb);
    memcpy(index_end, file + info->index_offset, info->index_sb);
    memcpy(normal_end, file + info->normal_offset, info->normal_sb);

    free(file);
    return HephResult::Success;
}

this function will simply walk the directory and take all of the files.
it will not recursivly search, and ALL files will be loaded. Even if they are not HMODL
Better make sure the only files in here are HMODL! 
HephResult
Meshes::load_hmodl_batch_path(const std::string& dir)
{
#ifdef HEPH_OSX
    #include <dirent.h>
    #include <errno.h>
    DIR *dir_stream = opendir(dir.c_str());
    if (!dir_stream)
    {
        HEPH_PRINT_ERROR("Unable to open directory.");
        return HephResult::Failure;
    }
    struct dirent *dir_entry;
    int errno_save = errno;
    while((dir_entry = readdir(dir_stream)) != NULL)
    {
        
    }
    closedir(dir_stream);
#if HEPH_VALIDATE
    if (errno_save != errno)
    {
        HEPH_PRINT_ERROR("Failed to crawl through full directory.");
        return HephResult::Failure;
    }
#endif
#endif 

#ifdef HEPH_WINDOWS

#endif
}

 this function will simply walk the directory and take all of the files.
it will not recursivly search, and ALL files will be loaded. Even if they are not HMODL
Better make sure the only files in here are HMODL! 
HephResult
Meshes::load_hmodl_batch_fd(const int fd)
{

}   
*/