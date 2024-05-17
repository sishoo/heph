#pragma once

#include <stdint.h>

#define HMODL_HEADER_SIZE_BYTES sizeof(HephHmodlHeader)

typedef struct
{
        uint32_t vertex_sb, vertex_offset;
        uint32_t index_sb, index_offset;
        uint32_t normal_sb, normal_offset;
} HephHmodlHeader;


