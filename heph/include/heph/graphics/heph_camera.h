#pragma once

#include <stdint.h>

typedef struct
{
        uint32_t __push_constant_padding;
        float frustum_vertices[8]; // start of push constant data
        float view_matrix[16];
} heph_camera_t;

void heph_camera_init(heph_camera_t *const c);
void heph_camera_destroy(heph_camera_t *const c);


void heph_camera_update_position(heph_camera_t *const c, float dx, float dy, float dz);
void heph_camera_update_rotation(heph_camera_t *const c);
