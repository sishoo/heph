#pragma once


typedef struct
{
        float view_matrix[16];
        float frustum_vertices[32];
} heph_camera_t;

void heph_camera_init(heph_camera_t *const c);
void heph_camera_destroy(heph_camera_t *const c);


void heph_camera_update_position(heph_camera_t *const c, float dx, float dy, float dz);
void heph_camera_update_rotation(heph_camera_t *const c);
