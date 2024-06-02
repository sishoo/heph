#include "include/graphics/scene.h"

#include "include/common/error.hpp"

#include <stdlib.h>

void heph_scene_init(heph_scene_t *const s)
{
        memset(s, 0, sizeof heph_scene_t);
}

void heph_scene_initialize_main_buffers(heph_scene_t *const s)
{
        /* Initialize geometry buffer */
        uint32_t geometry_buffer_size_bytes = ;

        VkBufferCreateInfo geometry_buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = geometry_buffer_size_bytes,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        HEPH_COND_ABORT_NE(
            vkCreateBuffer(
                r->ldevice,
                &geometry_buffer_create_info,
                NULL,
                &s->main_buffers[HEPH_SCENE_GEOMETRY_BUFFER_INDEX].handle),
            VK_SUCCESS);

        VkMemoryAllocateInfo geometry_buffer_memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = geometry_buffer_size_bytes,
            .memoryTypeIndex = r->geometry_buffer_memory_type_index};

        vkAllocateMemory(r->ldevice, NULL, NULL, &r->geometry_buffer_device_memory);

        /* Initialize object buffer */
        r->object_buffer_swap = 0;
        uint32_t object_buffer_size_bytes = 2 * nobjects;

        VkBufferCreateInfo object_buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = object_buffer_size_bytes,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};

        HEPH_COND_ABORT_NE(vkCreateBuffer(r->ldevice, &object_buffer_create_info, NULL, r->object_buffer), VK_SUCCESS);

        VkMemoryAllocateInfo object_buffer_memory_allocate_info = {
            .sType VK_STRUCTURE_TYPE_MEMORTY_ALLOCATE_INFO,
            .allocationSize = object_buffer_size_bytes,
            .memoryTypeIndex = r->object_buffer_memory_type_index};

        vkAllocateMemory(r->ldevice, NULL, NULL, &r->object_buffer_device_memory);

        /* Initialize draw buffer */
        uint32_t draw_buffer_size_bytes = nobjects * sizeof(VkDrawIndexedIndirectCommand);

        VkBufferCreateInfo draw_buffer_create_info = {
            .sType VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size =,
            .usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};

        HEPH_COND_ABORT_NE(vkCreateBuffer(r->ldevice, &draw_buffer_create_info, NULL, r->draw_buffer), VK_SUCCESS);

        VkMemoryAllocateInfo draw_buffer_memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = object_buffer_size,
            .memoryTypeIndex = r->draw_buffer_memory_type_index};
}

void heph_scene_populate_main_buffers(heph_scene_t *const s)
{
        for ()
}

void heph_scene_destroy(heph_scene_t *const s)
{
        free();
}

/* Returns light ID in scene */
uint32_t heph_scene_add_light(heph_scene_t *const s, heph_light_t *light)
{
        if (light == NULL)
        {
                HEPH_PRINT_ERROR("The light you are trying to add does not exist. (LightPtr == NULL)");
                return;
        }

        if (s->light_capacity <=->nlights)
        {
                s->lights = realloc(s->lights, s->light_capacity * 2);
                HEPH_COND_ABORT(s->lights, NULL);
        }

        memcpy(s->lights[s->nlights++], light, sizeof(heph_light_t));
}

void heph_scene_remove_light(heph_scene_t *const s, uint32_t id)
{
}

/* Returns object ID in scene */
uint32_t heph_scene_add_object(heph_scene_t *const s, heph_object_t *object)
{
        if (object == NULL)
        {
                HEPH_PRINT_ERROR("The object you are trying to add does not exist. (ObjectPtr == NULL)");
                return;
        }

        if (s->object_capacity <=->nobjects)
        {
                s->objects = realloc(s->objects, s->object_capacity * 2);
                HEPH_COND_ABORT(s->objects, NULL);
        }

        memcpy(s->objects[s->nobjects++], object, sizeof(heph_object_t));
}

void heph_scene_remove_object(heph_scene_t *const s, uint32_t id)
{
        if (s->nobjects <= id)
        {
                HEPH_PRINT_ERROR("Object ID outside range of possible objects. (ObjectID < 0 || NObjects < ObjectID)");
                return;
        }

        // TODO make this function
        abort();
}

uint32_t heph_scene_add_mesh(heph_scene_t *const s, heph_mesh_t *const mesh)
{
}

void heph_scene_remove_mesh(heph_scene_t *const s, uint32_t mesh_id)
{
}