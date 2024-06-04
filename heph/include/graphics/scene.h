#pragma once

#include <stdint.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "./camera.h"
#include "./light.h"
#include "./mesh.h"
#include "./gpu_buffer.h"
#include "./object.h"

#define HEPH_SCENE_GEOMETRY_BUFFER_INDEX 0
#define HEPH_SCENE_OBJECT_BUFFER_INDEX 1
#define HEPH_SCENE_DRAW_BUFFER_INDEX 2
#define HEPH_SCENE_TEXTURE_BUFFER_INDEX 3

typedef struct
{             
        uint32_t ncameras, camera_capacity;
        heph_camera_t *cameras; 

        uint32_t nlights, light_capacity;
        heph_light_t *lights;  

        uint32_t nmeshes, meshes_capacity;
        heph_mesh_t *meshes;

        uint32_t nobjects, objects_capacity;
        heph_object_t *objects;

        /* Scene buffers */
        uint32_t object_buffer_swap; // interpereted as a bool
        union
        {
                heph_gpu_buffer_t geometry_buffer, draw_buffer, object_buffer, texture_buffer;
                heph_gpu_buffer_t main_buffers[4];
        };


        float projection_matrix[16];
} heph_scene_t;

void heph_scene_init(heph_scene_t *const s);
void heph_scene_destroy(heph_scene_t *const s);

void heph_scene_initialize_main_buffers(heph_scene_t *const s);
void heph_scene_allocate_main_buffers(heph_scene_t *const s);
void heph_scene_populate_main_buffers(heph_scene_t *const s);

uint32_t heph_scene_add_light(heph_scene_t *const s, heph_light_t *const light);
void heph_scene_remove_light(heph_scene_t *const s, uint32_t light_id);

uint32_t heph_scene_add_mesh(heph_scene_t *const s, heph_mesh_t *const mesh);
void heph_scene_remove_mesh(heph_scene_t *const, uint32_t mesh_id);

uint32_t heph_scene_add_object(heph_scene_t *const s, heph_object_t *const object);
void heph_scene_remove_object(heph_scene_t *const s, uint32_t object_id);

