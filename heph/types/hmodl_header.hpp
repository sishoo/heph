#pragma once

#include <stdint.h>

struct HMODLHeader
{
        uint32_t vertex_sb, vertex_offset;
        uint32_t index_sb, index_offset;
        uint32_t normal_sb, normal_offset;
};
