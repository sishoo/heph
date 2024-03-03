
#pragma once

#include "renderer.hpp"

struct Hephaestus
{
        HephRenderer *renderer;
};

void init_hephaestus(Hephaestus *heph);
void shutdown_hephaestus(Hephaestus *heph);
