#pragma once

#include "utils/heph_error.hpp"

#include <stdio.h>

struct HephConfig
{

};

HephResult init_config(HephConfig *const config)
{
        FILE *fptr = fopen("config.txt", "rb");
        fread(config, sizeof(HephConfig), 1, fptr);
}