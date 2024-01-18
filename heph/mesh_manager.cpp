#include "mesh_manager.hpp"
#include "heph_utils.hpp"
#include "heph_defines.hpp"


#include <array>
#include <filesystem>
#include <fstream>


FileType 
MeshManager::check_file_type(const std::string& path)
{
    std::array<char, MAX_FILE_EXTENSION_SIZE> file_extension;
    std::string::const_iterator i = path.end();
    while (*i == '.' && i != path.begin())
        i--;
    memcpy(&file_extension, &*i, path.end() - i);   
    if (!strcmp(file_extension.data(), "hmodl"))
    {
        return FileType::Hmodl;
    }
    else if (!strcmp(file_extension.data(), "obj"))
    {
        return FileType::Obj;
    }
    return FileType::Unsupported;
}

HephResult
MeshManager::load(const std::string& path)
{
    FileType file_type = check_file_type(path);

    if (file_type == FileType::Unsupported)
    {
        heph_print_error("The file located at " << path << " is in an unsupported format.");
        return HephResult::Failure;
    }

    char *mapped_ptr = nullptr;

    /* Init the mapped memory for macos */
#ifdef HEPH_OSX


#endif
       


    /* Init the mapped memory for windows */
#ifdef HEPH_WINDOWS
    
    #include <windows.h>

    TCHAR *file_name = path.data();
    
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    HANDLE file_handle = CreateFile(
        file_name, 
        GENERIC_READ, 
        0, 
        nullptr, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        heph_print_error("Cannot map file at location: " << path << " to memory.");
        return HephResult::Failure;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle, &file_size))
    {
        heph_print_error("Getting the size of the file at location: " << path << " failed.");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    if (file_size.QuadPart == 0)
    {
        heph_print_error("The file at location: " << path << " is empty.");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    HANDLE map_handle = CreateFileMapping(
        file_handle,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (map_handle == 0)
    {
        heph_print_error("Failed to map the file located at: " << path << " to memory");
        CloseHandle(file_handle);
        return HephResult::Failure;
    }

    LPVOID base_ptr = MapViewOfFile(
        map_handle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (base_ptr == nullptr)
    {
        heph_print_error("Failed to get base ptr to mapped file memory. file path: " << path);
        CloseHandle(map_handle);
        CloseHandle(file_handle);
        return HephResult::Failure;
    }


    mapped_ptr = base_ptr;


#endif

    /* Do the stuff */

    































    /* Clean up the mapped memory for macos */
#ifdef HEPH_OSX

#endif

    /* Clean up the mapped memory for windows */
#ifdef HEPH_WINDOWS
    UnmapViewOfFile(base_ptr);
    CloseHandle(map_handle);
    CloseHandle(file_handle);
#endif





}





































template<typename T>
HEPH_RESULT 
MeshManager::load_into_buffer(const std::string& path, std::vector<T>& buffer)
{
    std::ifstream file(vertices, std::ios::binary | std::ios::ate);
    std::streamsize size = vertices_file.tellg();
    file.seekg(0, std::ios::beg);
    // maybe kind of a memory leak because if you are failing all your model loads then the cap might grow too much
    // even if reserve doesnt litteraly add the number passed in it can still grow out of control if a bunch of loads fail
    *buffer.reserve(size);
    if (!file.read(*buffer.data() + *buffer.size(), size))
    {
        HEPH_PRINT_ERROR("Failed to load the model data located at: " << path);
        return HEPH_FAILED;
    }

    return HEPH_SUCCESS;
}

HEPH_RESULT
MeshManager::calc_insert_vertex_normals(size_t vertices_start, size_t indices_start)
{
    std::vector<uint32_t> accum_lookup_table(vertices.size());

    auto i = indices.begin() + indices_start;
    for (size_t index : i)
    {
        
    }

}

HEPH_RESULT
MeshManager::load_hmodl(const std::string& dir)
{
    namespace fs = std::filesystem;
    std::string vertices_path = "";
    std::string indices_path = "";
    std::string vertex_normals_path = "";
    for (const auto& entry : fs::directory_iterator(dir))
    {
        auto path = entry.path();
        auto ext =  path.extension();
        if (".hmodlv" == ext)
        {
            vertices_path = path;
        } 
        else if (".hmodlf" == ext)
        {
            indices_path = path;
        }
        else if (".hmodlvn" == ext)
        {
            vertex_normals_path = path;
        }
    }
    if (vertices.empty() || indices.empty())
    {
        HEPH_PRINT_ERROR("Failed to load model located at: " << dir);
        return HEPH_FAILED;
    }

    size_t vertices_start = vertices.size();
    if (load_into_buffer(vertices_path, vertices) != HEPH_SUCCESS)
    {
        return HEPH_FAILED;
    }

    size_t indices_start = indices.size();
    if (load_into_buffer(indices_path, indices) != HEPH_SUCCESS)
    {
        return HEPH_FAILED;
    }
    
    if (vertex_normals_path.empty())
    {   
        calc_insert_vertex_normals(vertices_start, indices_start);
    }


}

/*
FileType 
MeshManager::check_model_type(const std::string& path)
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
    FileType type = check_file_type(path);

    std::optional<std::vector<float>> file_buffer;
    switch (type)
    {
        case FileType::HMODL:
        {
           file_buffer = load_into_buffer(path);
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

*/