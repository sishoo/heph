#pragma once

#include <stdint.h>

typedef struct
{       
        uint32_t vertex_start, index_start, nindices; // temp 
        uint32_t mesh_id, material_id;
        float model_matrix[16];
} heph_object_t;

void heph_object_init(heph_object_t *const o); 

void heph_object_update_position(heph_object_t *const o, float dx, float dy, float dz);
void heph_object_update_rotation(heph_object_t *const o);