#pragma once

#include <stdlib.h>

typedef struct 
{
        char *ptr;
        size_t sb;      
} heph_string_t;

void heph_type_string_init(heph_string_t *const cs, char *const ptr, const size_t size)
{
        cs->ptr = ptr;
        cs->sb = size;
}

void heph_type_string_destroy(heph_string_t *const cs)
{
        if (!cs->sb)
        {
                return;
        }
        cs->sb = 0;
        free(cs->ptr);
}       


