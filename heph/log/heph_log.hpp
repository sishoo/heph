#pragma once

#include "../utils/heph_defines.hpp"
#include "../utils/heph_error.hpp"

#if HEPH_VALIDATE
#define HEPH_DEV_NOTE(note) std::cout << "Note to self: " << note << " @ " << FILE_LINE_INFO_STRING << '\n';
#else
#define HEPH_DEV_NOTE(note)
#endif

#if HEPH_VALIDATE
#define HEPH_DEBUG_NOTE(note) std::cout << "Debug note: " << note << " @ " << FILE_LINE_INFO_STRING << '\n';
#else
#define HEPH_DEGUG_NOTE(note)
#endif
