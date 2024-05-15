#pragma once

#include "renderer.hpp"
#include "heph_thread_pool.hpp"

typedef struct
{
        char *name;
        
} HephApplicationContext;

typedef struct 
{
        HephThreadPool *application_thread_pool;

        pthread_t render_thread;
        HephRenderer *renderer;
} Hephaestus;

void hephaestus_init(Hephaestus *const heph);
void hephaestus_run(Hephaestus *const heph);
void hephaestus_shutdown(Hephaestus *const heph);
