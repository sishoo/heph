#pragma once

typedef struct
{
        union
        {
                float points[2];
                float x, y;
        };
} heph_vector2_t;

typedef struct
{
        union
        {
                float points[3];
                float x, y, z;
        };
} heph_vector3_t;