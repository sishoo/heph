#pragma once

#include "./heph_defines.hpp"
#include "./heph_error.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HEPH_OSX
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#endif

#ifdef HEPH_WINDOWS
#include <windows.h>
#include <winbase.h>
#endif 

HephResult
get_file_size_fd(const int fd, size_t *const size)
{
    if (!size || fd < 0)
    {
        HEPH_PRINT_ERROR("Invalid file descriptor OR ptr to size was null.");
        return HephResult::Failure;
    }

#ifdef HEPH_OSX
    struct stat status;
    if (fstat(fd, &status))
    {
        HEPH_PRINT_ERROR("Failed to get file size.");
        return HephResult::Failure;
    }
    *size = status.st_size;
#endif

#ifdef HEPH_WINDOWS
    struct _stati64 status;
    if (_fstati64(fd, &status))
    {
        HEPH_PRINT_ERROR("Failed to get file size.");
        return HephResult::Failure;
    }
    *size = status.st_size;
#endif

    return HephResult::Success;
}

HephResult
enumerate_directory_entries(const std::string &dir_path, std::vector<std::string> &entries)
{
#ifdef HEPH_OSX
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dir_path.c_str())) == NULL)
    {
        HEPH_PRINT_ERROR("Failed to open directory.");
        return HephResult::Failure;
    }
    while((ent = readdir(dir)) != NULL)
    {
        entries.push_back(ent->d_name);
    }
    closedir(dir);
    return HephResult::Success;
#endif

#ifdef HEPH_WINDOWS
    return HephResult::Failure;
#endif
}

// dont use this function then call stat yourself to do other stuff. 
// if you already have a file descriptor use that
HephResult
get_file_size_path(const std::string &path, size_t *const size)
{
#ifdef HEPH_WINDOWS
    #define stat _stati64
#endif

    if (!size)
    {
        HEPH_PRINT_ERROR("Size ptr was null.");
        return HephResult::Failure;
    }

    struct stat status;
    if (stat(path.c_str(), &status))
    {
        HEPH_PRINT_ERROR("Failed to get file size.");
        return HephResult::Failure;
    }
    *size = status.st_size;

     return HephResult::Success;

#undef stat
}

// prefered over map_file
void *
fopen_file(const std::string &path, size_t *const size, const char open_mode[10])
{
    if (!size || !open_mode)
    {
        HEPH_PRINT_ERROR("No size or open modes were provided.");
        return NULL;
    }
    FILE *fptr = fopen(path.c_str(), open_mode);
    if (!fptr)
    {   
        HEPH_PRINT_ERROR("File ptr was null.");
        return NULL;
    }

    void *file = malloc(*size);
    fread(file, *size, 1, fptr);
    fclose(fptr);
    return file;
}

// fopen is prefered
void *
map_file(const std::string &path, int *const fdp, size_t *const size)
{
    if (!fdp || !size)
    {
        HEPH_PRINT_ERROR("File descriptor ptr or file size ptr was null.")
        return NULL;
    }
        
#ifdef HEPH_OSX
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return NULL;
    if (get_file_size_fd(fd, size) == HephResult::Failure)
    {
        HEPH_PRINT_ERROR("Getting status of file at location: " << path << " failed.");
        close(fd);
        return NULL;
    }
    char* mapped_ptr = (char *)mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
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
    *size = file_size.QuadPart;
    return mapped_ptr;
#endif
}

// i know the file can be deleted after the time of calling this function
// i dont care
bool
file_exists(const std::string &path)
{
#ifdef HEPH_WINDOWS
    #define access _access
#endif

    return !(bool)access(path.c_str(), F_OK);

#undef access
}

HephResult
read_file_to(void *const dest, size_t *const size, const std::string &path)
{
#ifdef HEPH_WINDOWS
    #define open _open
    #define read _read
#endif

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        HEPH_PRINT_ERROR("Failed to open file.");
        return HephResult::Failure;
    }
    if (get_file_size_fd(fd, size) == HephResult::Failure)
        return HephResult::Failure;
    read(fd, dest, *size);

    return HephResult::Success;

#undef open
#undef read
}