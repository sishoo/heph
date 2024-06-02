#pragma once

#include <stdint.h>

typedef struct
{
        uint32_t __push_constant_padding;
        float view_matrix[16];
        float frustum[8];
} heph_camera_t;

heph_camera_t heph_camera_init(heph_camera_t *const info);
void heph_camera_destroy(heph_camera_t c);

void heph_camera_update_position(heph_camera_t c, float dx, float dy, float dz);
void heph_camera_update_rotation(heph_camera_t c);










