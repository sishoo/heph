#include "renderer.hpp"
#include "renderer.cpp"


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <stdio.h>
#include <string>


// clang++ -o main main.cpp -lglfw -lvulkan

int main() {

    const std::string name = "Bro";
    Renderer render = Renderer(name);


    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
