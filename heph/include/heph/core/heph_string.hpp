#pragma once

#include <stdlib.h>

typedef struct 
{
        char *ptr;
        size_t sb;      
} heph_string_t;

void heph_type_string_init(heph_string_t *const s, char *const ptr, size_t size)
{
        s->ptr = ptr;
        s->sb = size;
}

void heph_string_destroy(heph_string_t *const s)
{
        if (s->sb == NULL)
        {
                return;
        }
        s->sb = 0;
        free(s->ptr);
}       


