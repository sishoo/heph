#pragma once

#include "./heph_defines.hpp"
#include <iostream>

#define INFO_STRING " File: " << __FILE__ << " Line: " << __LINE__

#if HEPH_VALIDATE
    #define HEPH_PRINT_ERROR(error_msg) std::cerr <<  "Hephaestus non fatal (but noticable) error: " << error_msg << INFO_STRING << std::endl;
#else 
    #define HEPH_PRINT_ERROR(error_msg)
#endif

#define HEPH_THROW_FATAL_ERROR(err_msg) std::cerr << "Hephaestus Fatal Error: " << err_msg << std::endl; abort();

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