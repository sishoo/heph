#pragma once

#include "../utils/heph_defines.hpp"
#include "../utils/heph_error.hpp"

#if HEPH_VALIDATE
#define DEV_NOTE(note) std::cout << "Note to self: " << note << " @ " << INFO_STRING << '\n';
#else
#define DEV_NOTE(note)
#endif

#if HEPH_VALIDATE
#define DEBUG_NOTE(note) std::cout << "Debug note: " << note << " @ " << INFO_STRING << '\n';
#else
#define DEGUG_NOTE(note)
#endif