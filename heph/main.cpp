#include "renderer.hpp"
#include "renderer.cpp"
#include "meshes.hpp"
#include "meshes.cpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <iostream>

// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl
// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl -Wl,-rpath,/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib

// g++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -Werror

int 
main() {
    // auto m = Meshes::from_path("soifnsf");
    auto m = Meshes("sfsfsfs");
    auto r = Renderer(m);
    r.load_mesh_data();
    r.render_loop();

    

    return 0;
}
