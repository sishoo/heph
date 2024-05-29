#pragma once

#include <stdint.h>

typedef struct
{       
        uint32_t mesh_id, material_id;
        float model_matrix[16];
} heph_object_t;

void heph_object_init(heph_object_t *const o);