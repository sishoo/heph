#pragma once

#include <stdint.h>
#include <cstdlib>
#include <string>

typedef struct
{
    void *ptr;
    int fd;
    size_t sb;

    MappedFile() 
    {
        ptr = NULL;
        fd = -1;
        sb = 0;
    }

    static HephResult
    init_from_empty(MappedFile *file, const std::string& path)
    {
        if (!file)
            return HephResult::Failure;
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0)
            return HephResult::Failure;
        struct stat status;
        if (fstat(fd, &status) < 0)
        {   
            close(fd);
            return HephResult::Failure;
        }
        char* mapped_ptr = (char *)mmap(NULL, status.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped_ptr == MAP_FAILED)
        {
            close(fd);
            return HephResult::Failure;
        }   
        file->ptr = mapped_ptr;
        file->fd = fd;
        file->sb = status.st_size;
        return HephResult::Success;
    }

} MappedFile;

