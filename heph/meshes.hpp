#pragma once

#include "types/vertex.hpp"
#include "types/vec.hpp"


#include <vector>



#ifdef HEPH_WINDOWS
    #include <stdio.h>
    constexper MAX_OPEN_FILES = _getmaxstdio();
#endif

#ifdef HEPH_OSX
    

#endif

struct LoadingBatch
{
    

};

struct Meshes
{
    std::vector<Vertex> vertex_buffer;
    std::vector<uint32_t> index_buffer;
    std::vector<Vec3> vertex_normals;

    void load(const std::string& path);
    void load_batch(const std::vector<std::string>& paths);
};