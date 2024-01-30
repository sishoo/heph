#include "mesh_manager.hpp"

#include <array>
#include <filesystem>
#include <fstream>

void
MeshManager::set_prealloc_amount_bytes(uint32_t size)
{
    prealloc_amount_bytes = size;
}

MeshManager::MeshManager(uint32_t prealloc_size_bytes)
{
    
}

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
#ifdef HEPH_OSX

    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <cstdio>
    #include <cstdlib>
    #include <unistd.h>
    #include <fcntl.h>

    int file_desc = open(path.c_str(), O_RDONLY);
    struct stat status;
    if (fstat(file_desc, &status) < 0)
    {   
        HEPH_PRINT_ERROR("Getting status of file at location: " << path << " failed.");
        return HephResult::Failure;
    }

    char* mapped_ptr = (char *)mmap(nullptr, status.st_size, PROT_READ, MAP_PRIVATE, file_desc, 0);
    if (mapped_ptr == MAP_FAILED)
    {
        HEPH_PRINT_ERROR("Mapping the file to memory failed. File location: " << path);
        return HephResult::Failure;
    }

    *ptr_to_ptr = mapped_ptr;

    close(file_desc);
#endif

#ifdef HEPH_WINDOWS
    
    #include <windows.h>

    TCHAR *file_name = path.data();
    
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    HANDLE file_handle = CreateFile(
        file_name, 
        GENERIC_READ, 
        0, 
        nullptr, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        HEPH_PRINT_ERROR("Cannot map file at location: " << path << " to memory.");
        return HephResult::Failure;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle, &file_size))
    {
        HEPH_PRINT_ERROR("Getting the size of the file at location: " << path << " failed.");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    if (file_size.QuadPart == 0)
    {
        HEPH_PRINT_ERROR("The file at location: " << path << " is empty.");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    HANDLE map_handle = CreateFileMapping(
        file_handle,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (map_handle == 0)
    {
        HEPH_PRINT_ERROR("Failed to map the file located at: " << path << " to memory");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    LPVOID mapped_ptr = MapViewOfFile(
        map_handle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (mapped_ptr == nullptr)
    {
        HEPH_PRINT_ERROR("Failed to get base ptr to mapped file memory. file path: " << path);
        CloseHandle(map_handle);
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    *ptr_to_ptr = mapped_ptr;

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
    FileType file_type = check_file_type(path);

    if (file_type == FileType::Unsupported)
    {
        HEPH_PRINT_ERROR("The file located at: " << path << " is in an unsupported format.");
        return HephResult::Failure;
    }

    char *mapped_ptr = nullptr;
    if (map_file(&mapped_ptr, path) == HephResult::Failure)
    {
        HEPH_PRINT_ERROR("Failed to map model to memory. Path: " << path);
        return HephResult::Failure;
    }

    LoadingInfo *info = (LoadingInfo *)mapped_ptr;
    
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
}