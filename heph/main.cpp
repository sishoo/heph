

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <iostream>

// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl
// clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl -Wl,-rpath,/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib -Wall

// g++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -Werror

#include "hephaestus.hpp"
#include "hephaestus.cpp"

#include "hmodl/hmodl.hpp"

/*
        TODO
        make command submiting on another thread, so the main thread does have to do it
        render arbitrary mesh!!!!!!!!
        on application start, query a configuration file that says stuff
*/

int main()
{
        uint64_t size = 0b0000000000000000000001110110111100000000000000000000011101101111;
        if (heph_set_hmodl_xattr("cheburashka (2).obj", (char *)&size) == HephResult::Failure)
        {
                return -1;
        }

        Hephaestus heph{};
        hephaestus_init(&heph);
        hephaestus_run(&heph);
        hephaestus_shutdown(&heph);
        
        return 0;
}
