#pragma once

#include "./heph_type_vec.hpp"



/*
        TODO



        right now we only use vertex normals but when we make another 
        pipeline for face normals you gotta make two different structs



*/




typedef struct
{
        Vec3 pos;
        Vec3 normal;
} Vertex;