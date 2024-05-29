#pragma once

// #include "include/core/thread_pool.h"

#include "include/graphics/renderer.hpp"

typedef struct
{
        char *name;
        
} heph_application_context_t;

typedef struct 
{
        // heph_thread_pool_t thread_pool;

        shaderc_compiler_t shader_compiler;

        heph_renderer_t renderer;
} heph_instance_t;

void heph_instance_init(heph_instance_t *const h);
void heph_instance_run(heph_instance_t *const h);
void heph_instance_shutdown(heph_instance_t *const h);
