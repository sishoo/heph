#pragma once

#include <stdlib.h>

typedef struct 
{
        char *ptr;
        size_t size_bytes;      
} heph_string_t;

void heph_type_string_init(heph_string_t *const s, char *const ptr, size_t size)
{
        s->ptr = ptr;
        s->size_bytes = size;
}

void heph_string_destroy(heph_string_t *const s)
{
        if (s->ptr == NULL)
        {
                return;
        }
        s->size_bytes = 0;
        free(s->ptr);
}       


