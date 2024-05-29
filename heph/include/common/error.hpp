#pragma once

#include "./defines.hpp"

#include <stdio.h>

#define FILE_LINE_INFO_STRING " File: "__FILE__" Line: "__LINE__

#if HEPH_VALIDATE
#define HEPH_PRINT_ERROR(error_message) fprintf(stderr, "Hephaestus runtime error: " error_message "@" FILE_LINE_INFO_STRING "\n");
#else
#define HEPH_PRINT_ERROR(error_message)
#endif

#define HEPH_ABORT(error_message)                                                                         \
        do                                                                                                \
        {                                                                                                 \
                fprintf(stderr, "Hephaestus fatal error: " error_message "@" FILE_LINE_INFO_STRING "\n"); \
                abort();                                                                                  \
        } while (0);

/* if (predicate != condition) abort(); */
#define HEPH_COND_ABORT_NE(predicate, condition)                                              \
        do                                                                                 \
        {                                                                                  \
                if ((predicate) != (condition))                                               \
                {                                                                          \
                        fprintf("Hephaestus conditional abort: "#predicate"!="#condition" @ "FILE_LINE_INFO_STRING"\n");                                  \
                        abort();\
                }                                                                          \
        } while (0);

/* if (predicate == condition) abort(); */
#define HEPH_COND_ABORT(predicate, condition)                                                 \
        do                                                                                 \
        {                                                                                  \
                if ((predicate) == (condition))                                               \
                {                                                                          \
                        HEPH_ABORT(""); \
                }                                                                          \
        } while (0);

/* if (predicate != cond) return retval; */                 \
#define HEPH_COND_RETURN_NE(_predicate, _cond, _retval) \
        do                                                  \
        {                                                   \
                if ((_predicate) != (_cond))                \
                {                                           \
                        return _retval;                     \
                }                                           \
        } while (0);

/* if (predicate == cond) return retval; */              \
#define HEPH_COND_RETURN(_predicate, _cond, _retval) \
        do                                               \
        {                                                \
                if ((_predicate) == (_cond))             \
                {                                        \
                        return _retval;                  \
                }                                        \
        } while (0);
