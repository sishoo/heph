#include "hephaestus.hpp"

#include "utils/heph_defines.hpp"
#include "utils/heph_error.hpp"

#include "config.hpp"

#include "renderer.cpp"

void hephaestus_init(Hephaestus *const heph)
{
        heph->renderer = (HephRenderer *)HALLOC(sizeof(HephRenderer));
        memset(heph->renderer, 0, sizeof(HephRenderer));
        heph_renderer_init(heph->renderer, "BRO WINDOW", 800, 600);

        /*
        if (!init_gui())
        {
                HEPH_ABORT("Failed to init gui.")
        }
        */
}

void hephaestus_run(Hephaestus *const heph)
{
        HephRenderer *r = heph->renderer;
        GLFWwindow *window = heph->renderer->window;
        
        bool drawing = true;
        while (!glfwWindowShouldClose(window))
        {
                glfwPollEvents();

                int width, height;
                glfwGetWindowSize(window, &width, &height);
                if (width != r->window_width || height != r->window_height)
                {
                        heph_renderer_rebuild_swapchain(r, width, height);
                        r->window_width = width;
                        r->window_height = height;
                }

                drawing = glfwGetWindowAttrib(window, GLFW_FOCUSED);
                if (!drawing)
                {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                }

                heph_renderer_render_frame(r);
        }
}

void hephaestus_shutdown(Hephaestus *const heph)
{
        heph_renderer_destroy(heph->renderer);
        free(heph->renderer);
}