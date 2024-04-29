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


typedef struct
{
        uintptr_t handle; /* integer on macos/linux void * on windows */
        size_t sb;
        char *ptr;    
} HephMappedFile;

bool heph_file_map_file(HephMappedFile *const mapped, char *const path)
{
        if (path == NULL || mapped == NULL)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return false;
        }

#ifdef HEPH_OSX
        int fd = open(path, O_RDONLY);

        if (fd == -1)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return false;
        }

        struct stat status;
        if (fstat(fd, &status) == -1)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return false;
        }

        mapped->ptr = (char *)mmap(path, status.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);        
        if (mapped->ptr == MAP_FAILED)
        {
                HEPH_PRINT_ERROR("Failed to open file.");
                return false;
        }
#endif

#ifdef HEPH_WINDOWS
        HEPH_ABORT("BRO YOU FORGOT TO IMPLEMENT THIS HE HE HE HA.");
#endif

        return true;
}

bool heph_file_unmap_file(HephMappedFile *const mapped)
{
#ifdef HEPH_OXS
        return !munmap(mapped->ptr, mapped->sb);
#endif  
}




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
