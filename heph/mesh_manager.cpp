#include "mesh_manager.hpp"

#include <array>
#include <filesystem>
#include <fstream>


FileType 
MeshManager::check_file_type(const std::string& path)
{
    std::array<char, MAX_FILE_EXTENSION_SIZE> file_extension;
    std::string::const_iterator i = path.end();
    while (*i == '.' && i != path.begin())
        i--;
    memcpy(&file_extension, &*i, path.end() - i);   
    if (!strcmp(file_extension.data(), "hmodl"))
    {
        return FileType::Hmodl;
    }
    else if (!strcmp(file_extension.data(), "obj"))
    {
        return FileType::Obj;
    }
    return FileType::Unsupported;
}


HephResult
MeshManager::map_file(char **ptr_to_ptr, const std::string& path) 
{

#ifdef HEPH_WINDOWS
    
    

#endif

#ifdef HEPH_OSX
    munmap((void *)mapped_ptr, status.st_size);
#endif

#ifdef HEPH_WINDOWS
    UnmapViewOfFile(mapped_ptr);
    CloseHandle(map_handle);
    CloseHandle(file_handle);
#endif
}

HephResult
MeshManager::load(const std::string& path)
{
    
}