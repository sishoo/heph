#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <array>
#include <optional>

#include "loader.hpp"
#include "heph_utils.hpp"
#include "heph_defines.hpp"



std::optional<std::vector<char>> Loader::load_into_buffer(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> file_buffer(size);
    if (!file.read(file_buffer.data(), size))
    {
        return {};
    }
    return file_buffer;
}

std::optional<Mesh> Loader::load_hmodl(const std::vector<char>& data)
{
    
}

std::optional<Mesh> Loader::load(const std::string& path, uint32_t vertices_cursor, uint32_t indicies_cursor)
{






    switch(check_file_type(path))
    {
        case FileType::HMODL: 
        {
            return load_hmodl();
        }
        case FileType::OBJ:
        {

            return ;
        }
        default:
        {
            HEPH_PRINT_ERROR("The model trying to be loaded is in an unupported format.");
            return {};
        }     
    }
}


void convert_to_hmodl(std::vector<char>& obj_data)
{

}

Mesh 
Loader::generate_mesh(const std::vector<uint32_t>& file_buffer)
{
    Mesh mesh{};
    mesh.vertices_start = file_buffer[0];
    mesh.vertices_num = file_buffer[1];
    mesh.indices_start = file_buffer[2];
    mesh.indices_num = file_buffer[3];

    
}

