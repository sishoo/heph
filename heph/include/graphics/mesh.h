#pragma once

#include <stdint.h>

typedef struct
{
        char *path;
        uint32_t mat_id;

        float bouding_radius;
} heph_mesh_t;

void heph_mesh_init(heph_mesh_t *const m);