#pragma once

#include "heph_object.h"

#include <stdint.h>



void heph_object_init(heph_object_t *const o, uint32_t mesh_id, uint32_t material_id);

void heph_object_update_position(heph_object_t *const o, float dx, float dy, float dz);
void heph_object_update_rotation(heph_object_t *const o);