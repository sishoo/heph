#pragma once

#include <stdint.h>
#include <stdlib.h>


struct ModelFileInfo
{   
  
};  

struct ModelFile
{
    int fd;
    size_t size;
    void *data;
    ModelFileInfo *info;

    ModelFile() : fd(-1), size(0), data(NULL), info(NULL) {}
};

