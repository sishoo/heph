#pragma once

#include "./defines.h"

#include <stdio.h>

#if HEPH_VALIDATE
#define HEPH_PRINT_ERROR(error_message) fprintf(stderr, "Hephaestus runtime error: " error_message);
#else
#define HEPH_PRINT_ERROR(error_message)
#endif

#define HEPH_ABORT(error_message)                                           \
        do                                                                  \
        {                                                                   \
                fprintf(stderr, "Hephaestus fatal error: " #error_message); \
                abort();                                                    \
        } while (0);

#define HEPH_ASSERT(predicate, condition)                                                        \
        do                                                                                       \
        {                                                                                        \
                if (predicate == condition)                                                      \
                {                                                                                \
                        fprintf(stderr, "Hephaestus assert failed:" #predicate "==" #condition); \
                        abort();                                                                 \
                }                                                                                \
        } while (0);

#define HEPH_NASSERT(predicate, condition)                                                      \
        do                                                                                      \
        {                                                                                       \
                if (predicate != condition)                                                     \
                {                                                                               \
                        fprintf(stderr, "Hephaestus assert faild:" #predicate "!=" #condition); \
                        abort();                                                                \
                }                                                                               \
        } while (0);

