#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"
#include "utils/heph_defines.hpp"

#include <vector>
#include <span>
#include <stdio.h>

struct Meshes
{
    std::vector<const std::string> paths;

    HephResult queue_hmodl(const std::string& path);
    HephResult queue_hmodl_batch(const std::vector<const std::string>& paths);
    HephResult queue_hmodl_directory(const std::string& dir);
};  