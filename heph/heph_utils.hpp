#pragma once

#include <iostream>

// result
typedef enum HephResult {
    Success,
    Failure
};

inline void heph_print_error(char *error_msg)
{
    std::cerr << "Hephaestus negligalbe but noticable error: " << *error_msg << std::endl;
}

inline void heph_throw_error_unrecoverable(char *error_msg)
{
    std::cerr << "Hephaestus Unrecoverable Error: " << error_msg << std::endl; 
    abort();
}




