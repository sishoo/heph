#pragma once

#include <stdlib.h>

typedef struct 
{
        const char *ptr;
        const size_t sb;      
} HephConstString;

void heph_type_const_string_init(HephConstString *const cs, char *const ptr, const size_t size)
{
        cs->ptr = ptr;
        *(size_t *)cs->sb = size;
}

void heph_type_const_string_destroy(HephConstString *const cs)
{
        if (!cs->sb)
        {
                return;
        }
        *(size_t *)cs->sb = 0;
        free((void *)cs->ptr);
}       


