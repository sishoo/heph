#include "hephaestus.hpp"

#include "utils/heph_defines.hpp"
#include "utils/heph_error.hpp"

#include "config.hpp"

#include "renderer.hpp"
#include "renderer.cpp"

void hephaestus_init(Hephaestus *const heph)
{
        heph->renderer = (HephRenderer *)malloc(sizeof(HephRenderer));
        memset(heph->renderer, 0, sizeof(HephRenderer));
        heph_renderer_init(heph->renderer);

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
                // handle events
                // if minimize tab / alt tab stop drawing

                int width, height;
                glfwGetWindowSize(window, &width, &height);
                if (width != r->window_width || height != r->window_height)
                {
                        heph_renderer_rebuild_swapchain(r, width, height);
                }

                if (!drawing)
                {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                }

                // render()
        }
}

void hephaestus_shutdown(Hephaestus *const heph)
{
        heph_renderer_destroy(heph->renderer);
        free(heph->renderer);
}