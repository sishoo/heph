#pragma once

#include "./heph_defines.hpp"
#include "./heph_error.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <optional>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

HephResult heph_get_file_size_fd(const int fd, size_t *const size)
{
        if (!size || fd < 0)
        {
                HEPH_PRINT_ERROR("Invalid file descriptor OR ptr to size was null.");
                return HephResult::Failure;
        }

        struct stat status;
        if (fstat(fd, &status))
        {
                HEPH_PRINT_ERROR("Failed to get file size.");
                return HephResult::Failure;
        }
        *size = status.st_size;

        return HephResult::Success;
}

HephResult heph_enumerate_directory_entries(const std::string &dir_path, std::vector<std::string> &entries)
{
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(dir_path.c_str())) == NULL)
        {
                HEPH_PRINT_ERROR("Failed to open directory.");
                return HephResult::Failure;
        }
        while ((ent = readdir(dir)) != NULL)
        {
                entries.push_back(ent->d_name);
        }
        closedir(dir);
        return HephResult::Success;
}

HephResult heph_get_file_size_path(const std::string &path, size_t *const size)
{
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
}


char *heph_file_map_file(char *const path)
{
        char *ptr = NULL;
        if (path == NULL)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return NULL;
        }

#ifdef HEPH_OSX
        int fd = open(path, O_RDONLY);

        if (fd == -1)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return NULL;
        }

        struct stat status;
        if (fstat(fd, &status) == -1)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return NULL;
        }

        ptr = (char *)mmap(path, status.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);        
        if (ptr == MAP_FAILED)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return NULL;
        }
#endif

#ifdef HEPH_WINDOWS
        HEPH_ABORT("BRO YOU FORGOT TO IMPLEMENT THIS HE HE HE HA.");
#endif

        return ptr;
}

/*
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
*/

// i know the file can be deleted after the time of calling this function
// i dont care
inline bool heph_file_exists(const std::string &path)
{
        return !(bool)access(path.c_str(), F_OK);
}

HephResult heph_read_file_to(void *const dest, size_t *const size, const std::string &path)
{
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return HephResult::Failure;
        }
        if (heph_get_file_size_fd(fd, size) == HephResult::Failure)
                return HephResult::Failure;
        read(fd, dest, *size);

        return HephResult::Success;
}
