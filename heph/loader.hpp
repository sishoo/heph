#pragma once

#include <stdint.h>
#include <string>
#include <optional>

#include "heph_defines.hpp"
#include "mesh.hpp"


enum FileType {
    UNSUPPORTED = 0,
    OBJ = 1,
    HMODL = 2
};

class Loader {
public:


    Mesh get_loading_info(const std::string& path);
    
    FileType check_file_type(const std::string& path);
    std::optional<std::vector<char>> load_into_buffer(const std::string& path);
    std::optional<Mesh> load_hmodl(const std::vector<char>& data);
    std::vector<char> obj_to_hmodl(const std::vector<char>& data);
    
};