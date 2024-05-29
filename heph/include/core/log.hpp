#pragma once

#include "../common/defines.hpp"
#include "../common/error.hpp"

#if HEPH_VALIDATE
#define HEPH_DEBUG_NOTE(note) std::cout << "Debug note: " << note << " @ " << FILE_LINE_INFO_STRING << '\n';
#else
#define HEPH_DEGUG_NOTE(note)
#endif
