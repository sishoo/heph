#pragma once


/* Platform Defines */
#ifdef __APPLE__
#define HEPH_OSX
#elif _WIN32
#define HEPH_WINDOWS
#elif _WIN64
#define HEPH_WINDOWS
#else
#define HEPH_UNSUPPORTED_OS
#endif

/* Debug/validate mode */
#ifdef NDEBUG
#define HEPH_VALIDATE false
#else
#define HEPH_VALIDATE true
#endif

/* checks vulkan monitor layer availability */
#ifdef HEPH_WINDOWS
#if HEPH_VALIDATE
#define HEPH_MONITOR true
#endif
#endif

// max file extension size
// its actually 255 but just to be safe 260
#define MAX_FILE_EXTENSION_SIZE 260

#define HMODL_XATTR_NAME "components_tsb"
#define HMODL_XATTR_VALUE_SIZE sizeof("00000000")
