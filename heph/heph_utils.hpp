#pragma once

#include <iostream>

#include "heph_defines.hpp"


#if HEPH_VALIDATE
    #define HEPH_PRINT_ERROR(error_msg) std::cerr <<  "Hephaestus non fatal (but noticable) error: " << error_msg << std::endl;
#else 
    #define HEPH_PRINT_ERROR(error_msg)
#endif

#define HEPH_THROW_FATAL_ERROR(err_msg) std::cerr << "Hephaestus Fatal Error: " << err_msg << std::endl; abort();






