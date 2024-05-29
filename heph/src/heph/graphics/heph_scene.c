#include "heph_scene.h"

#include "heph_error.hpp"

#include <stdlib.h>


void heph_scene_init(heph_scene_t *const s)
{
        memset(s, 0, sizeof(heph_scene_t));        
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

        if (s->light_capacity <= ->nlights)
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

        if (s->object_capacity <= ->nobjects)
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