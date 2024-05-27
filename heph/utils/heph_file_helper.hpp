#pragma once

#include "./heph_defines.hpp"
#include "../types/heph_type_string.hpp"
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



bool heph_file_read_to_string(HephString *const s, char *const path)
{
        FILE *fptr = fopen(path, "r");
        HEPH_COND_ERR_RETURN(fptr, NULL, false);
        
        HEPH_COND_ERR_RETURN_NE(fseek(fptr, 0, SEEK_END), 0, false);

        long flen = 0;
        HEPH_COND_ERR_RETURN((flen = ftell(fptr)), -1, false);

        char *mem = (char *)HCALLOC(flen, sizeof(char));

        if (fread(mem, 1, flen, fptr) < flen)
        {
                HFREE(mem);
                return false;
        }
        
        s->ptr = mem;
        s->sb = flen;

        return true;
}