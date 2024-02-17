#include "renderer.hpp"
#include "renderer.cpp"
#include "meshes.hpp"
#include "meshes.cpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <iostream>

// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -Werror -fsanitize=address
// g++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -Werror

int 
main() {
    auto m = Meshes("sfsfsfsfsfsf");
    auto r = Renderer(m);
    r.load_mesh_data();
    r.render_loop();

    

    r.destroy();
    return 0;
}
