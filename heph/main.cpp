#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <assert.h>

/*
        DEBUG:
        clang++ -g -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl -Wl,-rpath,/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib -Wall -I/usr/local/Cellar/glfw/3.4/include -I/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/include -L/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib -L/usr/local/Cellar/glfw/3.4/lib


        RELEASE:
        clang++ -o main main.cpp -lglfw -lvulkan -std=c++20 -fsanitize=address -ldl -Wl,-rpath,/Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib -Wall -Ofast



*/


#include "hephaestus.hpp"
#include "hephaestus.cpp"

#include "hmodl/hmodl.hpp"

/*
        TODO
        make command submiting on another thread, so the main thread does have to do it
        render arbitrary mesh!!!!!!!!
        on application start, query a configuration file that says stuff


        make each entity in the game have a handle: player, enemy, dog, etc

        make a trait system thing

        heph_implement_trait_move(HephGameEntityHandle handle, function ptr to custom implementation);
        OR
        heph_trait_implement_move(HephGameEntityHandle handle, HEPH_TRAIT_GENERIC_MOVE);



        make multiple threads for command buffer recording then send to one thread for vkQueueSubmit();




        IF YOU DO THE #INCLUDE THING HOT RELOADING WILL BE A PAIN 
        maybe intentionally 'leak' the strings the #include is stored in so you only read off disk once

*/



/*
        probably outdated total number of recorded hours spent: 433
*/

int main()
{
        /*
        
                make the rendering engine now, worry about other stuff later
        */

        uint64_t size = 0b0000000000000000000001110110111100000000000000000000011101101111;
        if (heph_set_hmodl_xattr("cheburashka (2).obj", (char *)&size) == HephResult::Failure)
        {
                return -1;
        }

        Hephaestus heph{};
        hephaestus_init(&heph);
        hephaestus_run(&heph);
        hephaestus_shutdown(&heph);

        printf("\n\nHALLOC COUNT: %ld\n\n", HALLOC_COUNT);
        return 0;
}
