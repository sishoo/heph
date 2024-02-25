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
// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl -Wl,-rpath,/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib -codegen-units=1

// g++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -Werror

int main()
{
        auto m = Meshes::from_path("cheburashka (2).obj");
        auto r = Renderer(m);
        r.load_mesh_data();
        r.render_loop();

        in setup of renderer make shaders compile
        make handle stuff

        return 0;
}
