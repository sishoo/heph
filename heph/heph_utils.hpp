#pragma once

#include <stdio.h>

#define HEPH_THROW_ERROR_UNRECOVERABLE(err_msg) fprintf(stderr, "Hephaestus Unrecoverable Error: %s\n", err_msg); abort();








