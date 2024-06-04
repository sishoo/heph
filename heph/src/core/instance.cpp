#include "include/core/instance.hpp"

#include "include/common/defines.h"
#include "include/common/error.h"
#include "include/common/hmodl.hpp"


#include "include/graphics/renderer.hpp"
#include "src/graphics/renderer.cpp"

void heph_instance_verify_system_compatabiltiy(heph_instance_t *const h)
{       
// static bool HEPH_HMODL_XATTR_AVAILABLE = false;
// #ifdef HEPH_OSX
//        HEPH_HMODL_XATTR_AVAILABLE = (1 << pathconf("sample.hmodl", _PC_XATTR_SIZE_BITS)) - 1 >= HMODL_HEADER_SIZE_BYTES;
// #else

// #endif

}

void heph_instance_init(heph_instance_t *const h)
{
        memset(h, 0, sizeof(heph_instance_t));

        heph_instance_verify_system_compatabiltiy(h);
        
        // heph_thread_pool_init(h->thread_pool);
        heph_renderer_init(&h->renderer, "BRO WINDOW", 1920, 1080);
}

void heph_instance_run(heph_instance_t *const h)
{
        heph_renderer_t *const r = &h->renderer;
        GLFWwindow *const window = r->window;
        
        bool drawing = true;
        float last_time = 0.0;
        bool application_running = !glfwWindowShouldClose(window);
        while (application_running)
        {
                float current_time = glfwGetTime();
                float delta_time = current_time - last_time;
                last_time = current_time;

                glfwPollEvents();

                /* Check window resize */
                int width, height;
                glfwGetWindowSize(window, &width, &height);
                if (width != r->window_width || height != r->window_height)
                {
                        heph_renderer_handle_window_resize(r, width, height);
                }

                /* Check if window is in focus or skip drawing */
                drawing = glfwGetWindowAttrib(window, GLFW_FOCUSED);
                if (!drawing)
                {
                        struct timespec req = {
                                .tv_nsec = 5000
                        };
                        nanosleep(&req, NULL);
                        continue;
                }

                heph_renderer_render_frame(r);

                application_running = !glfwWindowShouldClose(window);
        }

        heph_instance_shutdown(h);
}

void heph_instance_shutdown(heph_instance_t *const h)
{
        #warning add thread pool shutdown eventually
        heph_renderer_destroy(&h->renderer);
}