#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <array>
#include <optional>

#include "loader.hpp"
#include "heph_utils.hpp"
#include "heph_defines.hpp"

FileType Loader::check_file_type(const std::string& path)
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

std::optional<Mesh> Loader::load(const std::string& path)
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

