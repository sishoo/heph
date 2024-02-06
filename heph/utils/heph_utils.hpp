#pragma once

#include <iostream>

#include "heph_defines.hpp"

#if HEPH_VALIDATE
    #define HEPH_PRINT_ERROR(error_msg) std::cerr <<  "Hephaestus non fatal (but noticable) error: " << error_msg << std::endl;
#else 
    #define HEPH_PRINT_ERROR(error_msg)
#endif

#define HEPH_THROW_FATAL_ERROR(err_msg) std::cerr << "Hephaestus Fatal Error: " << err_msg << std::endl; abort();


#if HEPH_VALIDATE
    #define NOTE(msg) std::cout << "Note to self: " << msg << std::endl;
#else
    #define NOTE(msg) 
#endif


void *
map_file(const std::string& path, uint32_t* fdp)
{
    if (!fdp)
        return NULL;
#ifdef HEPH_OSX
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <cstdio>
    #include <cstdlib>
    #include <unistd.h>
    #include <fcntl.h>

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return NULL;
    struct stat status;
    if (fstat(fd, &status) < 0)
    {   
        HEPH_PRINT_ERROR("Getting status of file at location: " << path << " failed.");
        close(fd);
        return NULL;
    }
    char* mapped_ptr = (char *)mmap(NULL, status.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_ptr == MAP_FAILED)
    {
        HEPH_PRINT_ERROR("Mapping the file to memory failed. File location: " << path);
        close(fd);
        return NULL;
    }   
    *fdp = fd;
    return mapped_ptr;
#endif

#ifdef HEPH_WINDOWS
    #include <windows.h>

    NOTE("Bruh be careful with this TCHAR from c str thing.");
    TCHAR *file_name = path.c_str();
    
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    HANDLE file_handle = CreateFile(
        file_name, 
        GENERIC_READ, 
        0, 
        NULL, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        HEPH_PRINT_ERROR("Cannot map file at location: " << path << " to memory.");
        return NULL;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle, &file_size))
    {
        HEPH_PRINT_ERROR("Getting the size of the file at location: " << path << " failed.");
        CloseHandle(file_handle);
        return NULL;
    }

    if (file_size.QuadPart == 0)
    {
        HEPH_PRINT_ERROR("The file at location: " << path << " is empty.");
        CloseHandle(file_handle);
        return NULL;
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
        return NULL;
    }

    LPVOID mapped_ptr = MapViewOfFile(
        map_handle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (mapped_ptr == NULL)
    {
        HEPH_PRINT_ERROR("Failed to get base ptr to mapped file memory. file path: " << path);
        CloseHandle(map_handle);
        CloseHandle(file_handle);
        return NULL;
    }

    // even though handle is a void ptr its 
    // actually used to look up in a table
    // like a file desc
    *fdp = file_handle;
    return mapped_ptr;
#endif
}




