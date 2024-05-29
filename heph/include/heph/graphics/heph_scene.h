#pragma once

#include <stdint.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "heph_camera.h"
#include "heph_camera.c"
#include "heph_light.h"
#include "heph_light.c"

typedef struct
{               
        uint32_t ncameras, camera_capacity;
        heph_camera_t *cameras; 

        uint32_t nlights, light_capacity;
        heph_light_t *lights;  

        uint32_t nmeshes, meshes_capacity;
        heph_mesh_t *meshes;

        /* Scene buffers */
        uint32_t nobjects, object_buffer_swap; // interpereted as a bool
        VkBuffer geometry_buffer, object_buffer, draw_buffer;
        VkDeviceMemory geometry_buffer_memory, object_buffer_memory, draw_buffer_memory;
        VkDescriptorSet geometry_buffer_descriptor, object_buffer_descriptor, draw_buffer_descriptor;
        const uint32_t geometry_buffer_memory_type_index, object_buffer_memory_type_index, draw_buffer_memory_type_index;
} heph_scene_t;

void heph_scene_init(heph_scene_t *const s);
void heph_scene_destroy(heph_scene_t *const s);

void heph_scene_allocate_scene_buffers(heph_scene_t *const s);
void heph_scene_generate_descriptor_sets(heph_scene_t *const s);
void heph_scene_populate_scene_buffers(heph_scene_t *const s);

void heph_scene_add_light(heph_scene_t *const s, heph_light_t *const l);
void heph_scene_remove_light(heph_scene_t *const s, uint32_t id);

void heph_scene_add_object(heph_scene_t *const s);
void heph_scene_remove_object(heph_scene_t *const s, uint32_t id);

