#pragma once

#include "./heph_defines.hpp"
#include <iostream>

#define INFO_STRING " File: " << __FILE__ << " Line: " << __LINE__

#if HEPH_VALIDATE
#define HEPH_PRINT_ERROR(error_msg) std::cerr << "Hephaestus runtime error: " << error_msg << " @ " << INFO_STRING << std::endl;
#else
#define HEPH_PRINT_ERROR(error_msg)
#endif

#define HEPH_ABORT(err_msg)                                                                              \
                                                                                                         \
        do                                                                                               \
        {                                                                                                \
                std::cerr << "Hephaestus Fatal Error: " << err_msg << " @ " << INFO_STRING << std::endl; \
                abort();                                                                                 \
        } while (0);

#if HEPH_VALIDATE
#define NOTE(msg) std::cout << "Note to self: " << msg << std::endl;
#else
#define NOTE(msg)
#endif

enum HephResult
{
        Failure,
        Success
};

inline HephResult operator&(HephResult a, HephResult b)
{
        return static_cast<HephResult>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline HephResult operator|(HephResult a, HephResult b)
{
        return static_cast<HephResult>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
