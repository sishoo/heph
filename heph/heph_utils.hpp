#pragma once

#include <iostream>

#define HEPH_THROW_ERROR_UNRECOVERABLE(bro) std::cerr << "Hephaestus Unrecoverable Error: " << bro << '\n'; abort();

#define HEPH_PRINT_ERROR(bro) std::cerr << "Hephaestus negligalbe but noticable error: " << bro << '\n';





