#include "model_file.hpp"

#include "../utils/heph_utils.hpp"
#include "../utils/heph_defines.hpp"

HephResult
load_hmodl(ModelFile* file, ModelFileInfo* header, const std::string& path)
{
    if (!file || !header)
    {
        HEPH_PRINT_ERROR("HMODLFile struct or HMODLHeader struct was null.")
        return HephResult::Failure;
    }

    int fd = 0;
    size_t size = 0;
    void *mapped_ptr;
    if (!(mapped_ptr = map_file(path, &fd, &size)))
    {
        HEPH_PRINT_ERROR("Failed to map the file: " << path << " to memory.");
        return HephResult::Failure;
    }

    header     = (ModelFileInfo *)mapped_ptr;
    file->fd   = fd;
    file->data = mapped_ptr;
    file->size = size;
    return HephResult::Success;
}



