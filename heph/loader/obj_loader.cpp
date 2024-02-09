#include "model_file.hpp"

#include "../utils/heph_utils.hpp"
#include "../utils/heph_defines.hpp"

void
parse_obj(char *mapped_ptr, size_t size, ModelFileInfo* info)
{

    NOTE("i is sizet and starts are uint32 might overflow in parseobj function obj_loader.cpp")

    uint32_t num_vertex = 0;
    uint32_t num_index = 0;
    uint32_t num_vertexnm = 0;
    uint32_t vertex_start = 0;
    uint32_t index_start = 0;
    uint32_t vertexnm_start = 0;
    
    size_t i = 0;
    while(i++ < size)
    {
        switch (mapped_ptr[i - 1])
        {
            case 'v':
                if (mapped_ptr[i] == 'n')
                {
                    if (!vertexnm_start)
                        vertexnm_start = i;
                    num_vertexnm++;
                }
                else 
                {
                    if (!vertex_start)
                    vertex_start = i;
                    num_vertex++;
                }
                break;
            case 'f':
                if (!index_start)
                    index_start = i;
                num_index++;
                break;
        }
    }

    if (num_vertexnm != num_vertex)



    


    index_start--;
    vertexnm_start--;
    vertex_start--;

    info->vertex_sb = num_vertex * sizeof(float);
    info->index_sb = num_index * sizeof(uint32_t);
    info->vertexnm_sb = num_vertexnm * sizeof(float);
    


}

HephResult
load_obj(ModelFile* file, ModelFileInfo* header, const std::string& path)
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

    parse_obj((char *)mapped_ptr, size);

    return HephResult::Success;
}



