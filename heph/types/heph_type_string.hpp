#pragma once

#include <stdlib.h>

typedef struct 
{
        char *ptr;
        size_t sb;      
} HephString;

void heph_type_string_init(HephString *const cs, char *const ptr, const size_t size)
{
        cs->ptr = ptr;
        cs->sb = size;
}

void heph_type_string_destroy(HephString *const cs)
{
        if (!cs->sb)
        {
                return;
        }
        cs->sb = 0;
        free(cs->ptr);
}       


