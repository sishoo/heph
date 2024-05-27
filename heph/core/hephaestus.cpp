#include "hephaestus.hpp"

#include "utils/heph_defines.hpp"
#include "utils/heph_error.hpp"
#include "types/hmodl_header.hpp"

#include "config.hpp"

#include "renderer.cpp"

void hephaestus_verify_system_compatabiltiy()
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

void hephaestus_init(Hephaestus *const heph)
{
        hephaestus_verify_system_compatabiltiy();
        

        // heph->application_thread_pool = (HephThreadPool *)HCALLOC(sizeof(HephThreadPool), 1);
        // heph_thread_pool_init(heph->application_thread_pool);
        heph->application_thread_pool = NULL;


        /*
                TODO
                put renderer on own thread
        */
        // HephRendererInitArgs heph_renderer_init_args = {};
        // heph_renderer_init_args.window_name = "BRO WINDOW";
        // heph_renderer_init_args.window_width = 800;
        // heph_renderer_init_args.window_height = 600;
        // pthread_create(&heph->render_thread, NULL, heph_renderer_init, (void *)&heph_renderer_init_args);
        heph->renderer = (HephRenderer *)HCALLOC(sizeof(HephRenderer), 1);
        memset(heph->renderer, 0, sizeof(HephRenderer));
        heph_renderer_init(heph->renderer, "BRO WINDOW", 800, 600, heph->application_thread_pool);
        
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
        
        float last_time = 0.0;
        bool drawing = true;
        uint64_t frame_num = 0;
        while (!glfwWindowShouldClose(window))
        {
                printf("\n\n==== FRAME: %llu ====\n\n", frame_num);
                HEPH_DEBUG_NOTE("frame time: " << last_time);
                float time = glfwGetTime();
                float delta_time = time - last_time;
                last_time = time;


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

                heph_renderer_render_frame(r, delta_time);
                frame_num++;
        }
}

void hephaestus_shutdown(Hephaestus *const heph)
{
        heph_renderer_destroy(heph->renderer);
        free(heph->renderer);
}