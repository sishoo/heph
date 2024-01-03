#pragma once


// platform stuff
#ifdef __APPLE__
    #define HEPH_OSX
#elif _WIN32
    #define HEPH_WINDOWS
#elif _WIN64
    #define HEPH_WINDOWS
#else 
    #define HEPH_UNSUPPORTED_OS
#endif


// validation layers???
#ifdef NDEBUG
    #define HEPH_VALIDATE false
#else
    #define HEPH_VALIDATE true
#endif


// monitor layer???
#ifdef HEPH_WINDOWS
    #if HEPH_VALIDATE
        #define HEPH_MONITOR
    #endif
#endif
