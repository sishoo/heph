#include "include/core/instance.hpp"

#include "include/common/defines.hpp"
#include "include/common/error.hpp"
#include "include/common/hmodl.hpp"


#include "include/graphics/renderer.hpp"
#include "src/graphics/renderer.cpp"

void heph_instance_verify_system_compatabiltiy(heph_instance_t *const h)
{       
        /*
                Because of tagged pointer nonsense the program will 
                only work if the address space 48 bits
        */
        assert(sizeof(uint64_t) == sizeof(uintptr_t));

#ifdef __STDC_NO_ATOMICS__
        HEPH_ABORT("Your computer does not support atomics.");
#endif

static bool HEPH_HMODL_XATTR_AVAILABLE = false;
#ifdef HEPH_OSX
       HEPH_HMODL_XATTR_AVAILABLE = (1 << pathconf("sample.hmodl", _PC_XATTR_SIZE_BITS)) - 1 >= HMODL_HEADER_SIZE_BYTES;
#else

#endif

}

void heph_instance_init(heph_instance_t *const h)
{
        memset(h, 0, sizeof(heph_instance_t));

        heph_instance_verify_system_compatabiltiy(h);
        
        heph_thread_pool_init(h->thread_pool);
        heph_renderer_init(h->renderer, "BRO WINDOW", 1920, 1080, NULL);
}

void heph_instance_run(heph_instance_t *const h)
{
        heph_renderer_t *const r = heph->renderer;
        GLFWwindow *const window = heph->renderer->window;
        
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
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                }

                heph_renderer_render_frame(r);

                application_running = !glfwWindowShouldClose(window);
        }

        heph_instance_shutdown(h);
}

void heph_instance_shutdown(heph_instance_t *const h)
{
        heph_renderer_destroy(heph->renderer);
}