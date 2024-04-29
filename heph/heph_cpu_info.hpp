#pragma once


/*
        TODO
        make a build script that collects these values
*/


// sysctl -a | grep hw.logicalcpu_max
#define HEPH_CPU_INFO_NTHREADS 4

// sysctl -a | grep hw.cachelinesize
#define HEPH_CPU_INFO_CACHE_LINE_SIZE_BYTES 64

