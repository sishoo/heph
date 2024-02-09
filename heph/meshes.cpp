#include "meshes.hpp"


#include "utils/heph_utils.hpp"
#include "types/hmodl_header.hpp"


HephResult
Meshes::load_hmodl(const std::string& path)
{
#if HEPH_OSX
    #include <sys/mman.h>
    int fd = -1;
    size_t size = 0;
    void *mapped_ptr = map_file(path, &fd, &size);
    madvise(mapped_ptr, size, MADV_SEQUENTIAL);
#endif 




    HMODLHeader *info = (HMODLHeader *)mapped_ptr;
    





}