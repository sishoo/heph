#pragma once

#include <string>

#include "../utils/heph_defines.hpp"

typedef struct
{
    uint32_t vertex_num, index_num, vn_num;
    uint32_t vertex_sb, index_sb, vn_sb;
} HMODLloadingInfo;

typedef struct 
{



    
    HephResult load(const std::string& path);

} HMODLloader;