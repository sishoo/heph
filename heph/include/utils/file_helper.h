#pragma once

#include "include/common/defines.h"
#include "include/core/string.h"
#include "include/common/error.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

bool heph_file_read_to_string(heph_string_t *const s, char *const path)
{
        FILE *fptr = fopen(path, "r");
        if (fptr == NULL)
                return false;
        

        if (!fseek(fptr, 0, SEEK_END))
                return false;

        long flen = ftell(fptr);        
        if (flen == -1)
                return false;

        char *memory = (char *)HCALLOC(flen, sizeof(char));

        if (fread(memory, 1, flen, fptr) < flen)
        {
                HFREE(memory);
                return false;
        }
        
        s->ptr = memory;
        s->size_bytes = flen;

        return true;
}