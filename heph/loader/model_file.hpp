#pragma once

#include <stdint.h>
#include <stdlib.h>


struct ModelFileInfo
{   
    uint32_t vertex_sb, vertex_offset;
    uint32_t index_sb, index_offset;
    uint32_t vertexnm_sb, vertexnm_offset;
};  

struct ModelFile
{
    int fd;
    size_t size;
    void *data;
    ModelFileInfo *info;

    ModelFile() : fd(-1), size(0), data(NULL), info(NULL) {}
};

