#pragma once

#include "./heph_defines.hpp"
#include <iostream>

#define FILE_LINE_INFO_STRING " File: " << __FILE__ << " Line: " << __LINE__

#if HEPH_VALIDATE
#define HEPH_PRINT_ERROR(_error_msg) std::cerr << "Hephaestus runtime error: " << _error_msg << " @ " << FILE_LINE_INFO_STRING << std::endl;
#else
#define HEPH_PRINT_ERROR(_error_msg)
#endif

#define HEPH_ABORT(_err_msg)                                                                                        \
        do                                                                                                          \
        {                                                                                                           \
                std::cerr << "Hephaestus Fatal Error: " << _err_msg << " @ " << FILE_LINE_INFO_STRING << std::endl; \
                abort();                                                                                            \
        } while (0);

/* if (predicate != cond) abort(); */
#define HEPH_COND_ABORT_NE(_predicate, _cond)                                              \
        do                                                                                 \
        {                                                                                  \
                if ((_predicate) != (_cond))                                               \
                {                                                                          \
                        HEPH_ABORT("Hephaestus condition abort: Predicate != condition."); \
                }                                                                          \
        } while (0);

/* if (predicate == cond) abort(); */
#define HEPH_COND_ABORT(_predicate, _cond)                                                 \
        do                                                                                 \
        {                                                                                  \
                if ((_predicate) == (_cond))                                               \
                {                                                                          \
                        HEPH_ABORT("Hephaestus condition abort: Predicate != condition."); \
                }                                                                          \
        } while (0);

/* if (predicate != cond) return retval; */                 \
#define HEPH_COND_ERR_RETURN_NE(_predicate, _cond, _retval) \
        do                                                  \
        {                                                   \
                if ((_predicate) != (_cond))                \
                {                                           \
                        return _retval;                     \
                }                                           \
        } while (0);

/* if (predicate == cond) return retval; */              \
#define HEPH_COND_ERR_RETURN(_predicate, _cond, _retval) \
        do                                               \
        {                                                \
                if ((_predicate) == (_cond))             \
                {                                        \
                        return _retval;                  \
                }                                        \
        } while (0);

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
