


#include "mesh_manager.hpp"


FileType 
MeshManager::check_file_type(const std::string& path)
{
    std::array<char, MAX_FILE_EXTENSION_SIZE> file_extension;
    std::string::const_iterator i = path.end();
    while (i != path.begin() && *i == '.')
        i--;
    memcpy(&file_extension, &*i, path.end() - i);   
    if (!strcmp(file_extension.data(), "hmodl"))
    {
        return FileType::HMODL;
    }
    else if (!strcmp(file_extension.data(), "obj"))
    {
        return FileType::OBJ;
    }
    return FileType::UNSUPPORTED;
}

HEPH_RESULT 
MeshManager::load_mesh(const std::string& path)
{
    FileType type = mesh_loader.check_file_type(path);

    std::optional<std::vector<uint32_t>> file_buffer;
    switch (type)
    {
        case FileType::HMODL:
        {
           file_buffer = mesh_loader.load_into_buffer(path);
           break;
        }
        case FileType::OBJ:
        {
            std::optional<std::vector<char>> obj_data = mesh_loader.load_obj(path);
            file_buffer = hmodl_from_obj(obj_data);
            create_hmodl_file(file_buffer);
            break;
        }
        default:
        {
            HEPH_PRINT_ERROR("Loading the mesh at" << path << "failed because file format is not supported. Supported formats: OBJ, HMODL");
            return HEPH_FAILED;
        }
    }

    if (!file_buffer)
    {
        HEPH_PRINT_ERROR("Loading the mesh at" << path << "failed.");
        return HEPH_FAILED;
    }
    
    Mesh mesh = mesh_loader.get_loading_info(&*file_buffer);
    uint32_t size_of_vertices = info.vertices_num * sizeof(float);
    uint32_t size_of_indices  = info.indices_num * sizeof(uint32_t);

    vertices.reserve(size_of_vertices);
    indices.reserve(size_of_indices);
    object_strides.push_back(size_of_indices);

    memcpy(vertices[vertices_cursor], file_buffer.data() + HMODL_DATA_OFFSET, size_of_vertices);
    memcpy(indices[indices_cursor], file_buffer.data() + HMODL_DATA_OFFSET + size_of_vertices, size_of_indices);

    vertices_cursor += size_of_vertices;
    indices_cursor  += size_of_indices;

    return HEPH_SUCCESS;
}   