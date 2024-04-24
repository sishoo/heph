#pragma once

#include "renderer.hpp"
#include "heph_thread_pool.hpp"

struct Hephaestus
{
        HephThreadPool *application_thread_pool;

        pthread_t render_thread;
        HephRenderer *renderer;
};

void hephaestus_init(Hephaestus *const heph);
void hephaestus_run(Hephaestus *const heph);
void hephaestus_shutdown(Hephaestus *const heph);
