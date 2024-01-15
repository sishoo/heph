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


// result
#define HEPH_RESULT uint32_t
// instead of checking whether something failed its recommended to check wether it did !HEPH_SUCCESS
// because its 1 and 0 and stuff
#define HEPH_SUCCESS 1
#define HEPH_FAILED 0 



// model cache path
#ifdef HEPH_OSX
    #define HEPH_MODEL_CACHE "./model_cache"
#elif HEPH_WINDOWS
    #define HEPH_MODEL_CACHE "./model_cache"
#else
    #define HEPH_NO_CACHE
#endif


// max file extension size
// its actually 255 but just to be safe 260
#define MAX_FILE_EXTENSION_SIZE 260
#define HMODL_DATA_OFFSET sizeof(uint32_t * 4)