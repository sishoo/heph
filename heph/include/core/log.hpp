#pragma once

#include "../common/defines.h"
#include "../common/error.h"

#if HEPH_DEBUG
#define HEPH_DEBUG_NOTE(note) std::cout << "Debug note: " << note << " @ " << FILE_LINE_INFO_STRING << '\n';
#else
#define HEPH_DEGUG_NOTE(note)
#endif
