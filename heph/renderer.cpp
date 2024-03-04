#include "renderer.hpp"

#include "utils/heph_error.hpp"
#include "log/heph_log.hpp"
#include "utils/heph_utils.hpp"
#include "meshes.cpp"

#include "bootstrap/VkBootstrap.cpp"

#include <chrono>
#include <thread>

#define REQUIRED_QUEUE_FAMILY_BITFLAGS (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)
#define REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)

#define VERTEX_SHADER_PATH "heph\\shader\\culler.comp"
#define FRAGMENT_SHADER_PATH "heph\\shader\\fragment.frag"
#define CULLER_SHADER_PATH "heph\\shader\\vertex.vert"

static void get_buffer_memory_requirements(VkDevice ldevice, VkBuffer buffer, VkMemoryRequirements2 &requirements)
{
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        VkBufferMemoryRequirementsInfo2 info;
        info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        info.pNext = NULL;
        info.buffer = buffer;
        vkGetBufferMemoryRequirements2(ldevice, &info, &requirements);
}

static uint32_t get_memory_type_index(VkPhysicalDevice pdevice)
{
        VkPhysicalDeviceMemoryProperties2 pdevice_properties = {};
        pdevice_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(pdevice, &pdevice_properties);

        uint32_t i = 0;
        for (VkMemoryType type : pdevice_properties.memoryProperties.memoryTypes)
        {
                if ((type.propertyFlags & REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS) == REQUIRED_DATA_BUFFER_MEMORY_TYPE_BITFLAGS)
                {
                        return i;
                }
                i++;
        }
        HEPH_ABORT("Cannot find adequate memory type.");
}

static void record_command_buffer(VkCommandBuffer buffer, VkQueue queue, auto commands)
{
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(buffer, &info);
        commands(buffer, queue);
        vkEndCommandBuffer(buffer);
}

static void enqueue_command_buffer(VkCommandBuffer buffer, VkQueue queue, VkFence fence)
{
        VkCommandBufferSubmitInfoKHR command_info = {};
        command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_info.commandBuffer = buffer;

        VkSubmitInfo2 submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.pCommandBufferInfos = &command_info;

        if (vkQueueSubmit2(queue, 1, &submit_info, fence) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to enqueue command buffer.");
        }
}

void heph_renderer_init(HephRenderer *const r, const std::string &window_name, int width, int height)
{
        heph_renderer_init_instance(r);

        r->window_width = width;
        r->window_height = height;
        heph_renderer_init_window(r, window_name);

        heph_renderer_init_surface(r);
        heph_renderer_init_pdevice(r);
        heph_renderer_init_ldevice(r);
        heph_renderer_init_queue(r);

        heph_renderer_init_swapchain(r);
        heph_renderer_aquire_swapchain_images(r);

        heph_renderer_init_command_pool(r);
        heph_renderer_allocate_command_buffers(r);

        /*

                TODO
                when initing the command buffer make sure to do the stuff for each frame data

                also think about the mutlithread thing
        */

        heph_renderer_init_frame_render_infos(r);

        heph_renderer_init_sync_structures(r);

        // heph_renderer_init_helper_threads(r);
}

void heph_renderer_init_instance(HephRenderer *const r)
{
        vkb::InstanceBuilder builder;
        auto res = builder
                       .set_app_name("Hephaestus Renderer")
                       .set_engine_name("Hephaestus Engine")
                       .require_api_version(1, 3, 0)
                       .use_default_debug_messenger()
                       .request_validation_layers()
                       .build();
        if (!res)
        {
                HEPH_ABORT("Unable to create Vulkan Instance: " << res.error().message());
        }
        r->vkb_instance = res.value();
        r->instance = r->vkb_instance.instance;
}

void heph_renderer_init_window(HephRenderer *const r, const std::string &name)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if ((r->window = glfwCreateWindow(r->window_width, r->window_height, name.c_str(), NULL, NULL)) == NULL)
        {
                HEPH_ABORT("Unable to create window.");
        }
}

void heph_renderer_init_surface(HephRenderer *const r)
{
        if (glfwCreateWindowSurface(r->instance, r->window, NULL, &r->surface) != VK_SUCCESS)
        {
                HEPH_ABORT("Unable to create surface.");
        }
}

void heph_renderer_init_pdevice(HephRenderer *const r)
{
        printf("\n\n***DONT DELETE THIS PRINTF UNTIL YOU FIX THE DAMN PICK FIRST DEVICE UNCONDITIONALLY.***\n\n");
        vkb::PhysicalDeviceSelector selector(r->vkb_instance);
        auto res = selector
                       .set_surface(r->surface)
                       .select_first_device_unconditionally()
                       .select();
        if (!res)
        {
                HEPH_ABORT("Unable to find a physical device to use: " << res.error().message());
        }
        r->vkb_pdevice = res.value();
        r->pdevice = r->vkb_pdevice.physical_device;
}

void heph_renderer_init_ldevice(HephRenderer *const r)
{
        VkPhysicalDeviceSynchronization2Features sync_features = {};
        sync_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync_features.synchronization2 = VK_TRUE;

        vkb::DeviceBuilder builder{r->vkb_pdevice};
        auto res = builder
                       .add_pNext(&sync_features)
                       .build();
        if (!res)
        {
                HEPH_ABORT("Unable to create logical device: " << res.error().message());
        }
        r->vkb_ldevice = res.value();
        r->ldevice = r->vkb_ldevice.device;
}

void heph_renderer_init_queue(HephRenderer *const r)
{
        uint32_t i = 0;
        for (VkQueueFamilyProperties props : r->vkb_pdevice.get_queue_families())
        {
                VkQueueFlags flags = props.queueFlags;
                if ((flags & REQUIRED_QUEUE_FAMILY_BITFLAGS) == REQUIRED_QUEUE_FAMILY_BITFLAGS)
                {
                        goto found;
                }
                i++;
        }
        HEPH_ABORT("Unable to find suitable queue for Hephaestus to use");
found:
        r->queue_family_index = i;
        vkGetDeviceQueue(r->ldevice, i, 0, &(r->queue));
}

void heph_renderer_init_swapchain(HephRenderer *const r)
{
        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder
                       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .build();
        if (!res)
        {
                HEPH_ABORT("Unable to create swapchain: " << res.error().message());
        }
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;
}

void heph_renderer_aquire_swapchain_images(HephRenderer *const r)
{
        if (vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->swapchain_image_count, NULL) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to get the number of available swapchain images.");
        }

        r->swapchain_images = (VkImage *)calloc(sizeof(VkImage), r->swapchain_image_count);

        if (vkGetSwapchainImagesKHR(r->ldevice, r->swapchain, &r->swapchain_image_count, r->swapchain_images) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to aquire swapchain presentation images.");
        }

        for (uint32_t i = 0; i < r->swapchain_image_count; i++)
        {
                std::cout << r->swapchain_images[i] << '\n';
        }
}

void heph_renderer_init_command_pool(HephRenderer *const r)
{
        /*
                TODO
                we are using the command reset bit but int he future when we write commands every frame we need to change that
        */
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.queueFamilyIndex = r->queue_family_index;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(r->ldevice, &info, NULL, &r->command_pool) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to allocate command pool.");
        }
}

void heph_renderer_allocate_command_buffers(HephRenderer *const r)
{
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandBufferCount = 1;
        info.commandPool = r->command_pool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(r->ldevice, &info, &r->command_buffer) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to allocate command buffers");
        }
}

void heph_renderer_allocate_data_buffer(HephRenderer *const r)
{
        VkBufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = heph_meshes_size_b(r->meshes);
        DEBUG_NOTE("Allocating " << heph_meshes_size_b(r->meshes) << " amount of bytes to data buffer.");
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT_KHR;

        VkResult res = vkCreateBuffer(r->ldevice, &info, NULL, &r->data_buffer);
        if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        {
                HEPH_ABORT("Out of GPU memory.");
        }
        else if (res != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to create vkBuffer vertex buffer.");
        }

        VkMemoryRequirements2 requirements = {};
        get_buffer_memory_requirements(r->ldevice, r->data_buffer, requirements);

        uint32_t index = get_memory_type_index(r->pdevice);

        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.memoryTypeIndex = index;
        allocate_info.allocationSize = heph_meshes_size_b(r->meshes);

        vkAllocateMemory(r->ldevice, &allocate_info, NULL, &r->data_buffer_memory);
        if (vkBindBufferMemory(r->ldevice, r->data_buffer, r->data_buffer_memory, 0) != VK_SUCCESS)
        {
                HEPH_ABORT("Binding buffer memory failed.");
        }
}

void heph_renderer_load_mesh_data(HephRenderer *const r)
{
        void *ptr = NULL;

        if (vkMapMemory(r->ldevice, r->data_buffer_memory, 0, heph_meshes_size_b(r->meshes), 0, &ptr) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        if (ptr == NULL)
        {
                HEPH_ABORT("Failed to map memory.");
        }

        heph_meshes_write(r->meshes, static_cast<char *>(ptr));

        vkUnmapMemory(r->ldevice, r->data_buffer_memory);
}

void heph_renderer_rebuild_swapchain(HephRenderer *const r, int width, int height)
{
        /*
                TODO
                using vkb for the swapchain recreation might not be good i dont know
        */

        vkDeviceWaitIdle(r->ldevice);

        vkb::destroy_swapchain(r->vkb_swapchain);

        vkb::SwapchainBuilder builder{r->vkb_ldevice};
        auto res = builder.build();
        if (!res)
        {
                HEPH_ABORT("recreate swapchain: " << res.error().message());
        }
        r->vkb_swapchain = res.value();
        r->swapchain = r->vkb_swapchain.swapchain;

        heph_renderer_aquire_swapchain_images(r);
}

void heph_renderer_init_sync_structures(HephRenderer *const r)
{
        VkSemaphoreCreateInfo next_image_acquired_semaphore_info = {};
        next_image_acquired_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (vkCreateSemaphore(r->ldevice, &next_image_acquired_semaphore_info, NULL, &r->next_image_acquired_semaphore) != VK_SUCCESS)
        {
                HEPH_ABORT("A sync structure vital to execution was unable to be made.");
        }
}

void heph_renderer_init_frame_render_infos(HephRenderer *const r)
{
        VkSemaphoreCreateInfo render_complete_semaphore_info = {};
        render_complete_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo render_complete_fence_info = {};
        render_complete_fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        render_complete_fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        r->frame_render_infos = (HephFrameRenderInfo *)HALLOC(sizeof(HephFrameRenderInfo) * r->swapchain_image_count);

        for (uint32_t i = 0; i < r->swapchain_image_count; i++)
        {
                if (vkCreateSemaphore(r->ldevice, &render_complete_semaphore_info, NULL, &r->frame_render_infos[i].render_complete_semaphore) != VK_SUCCESS)
                {
                        HEPH_ABORT("Frame number " << i << "'s render complete semaphore was unable to be made.");
                }

                if (vkCreateFence(r->ldevice, &render_complete_fence_info, NULL, &r->frame_render_infos[i].render_complete_fence) != VK_SUCCESS)
                {
                        HEPH_ABORT("Frame number " << i << "'s render complete fence was unable to be made.");
                }
        }
}

// im fairly certain i dont even need to make the thread make a commpand pool or anything. just give it the handle to the
// cmd buffer and have it record;

// static void command_buffer_recorder_thread_start_routine()
// {

//         /*
//         TODO init the command pool for the thread;
//         */
//         VkCommandPoolCreateInfo command_pool_info = {};
//         command_pool_info
// }

// void heph_renderer_init_helper_threads(HephRenderer *const r)
// {
//         r->helper_threads = (pthread_t *)HALLOC(sizeof(pthread_t));

//         pthread_create(r->helper_threads + HEPH_RENDERER_COMMAND_BUFFER_RECORDING_THREAD_INDEX, NULL, &command_buffer_recorder_thread_start_routine, NULL);

// }

static void translate_image_layout(VkCommandBuffer buffer, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
{
        VkImageMemoryBarrier2 image_barrier = {};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        /*
                TODO:
                its only a good idea to use all commands bit when youre ***NOT*** transitioning an image
                probably get rid of this function in favor of a more fine grained command bits control
        */
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        image_barrier.oldLayout = current_layout;
        image_barrier.newLayout = new_layout;

        VkImageSubresourceRange sub_range = {};
        sub_range.baseMipLevel = 0;
        sub_range.levelCount = 1;
        sub_range.baseArrayLayer = 0;
        sub_range.layerCount = 1;
        /*
                TODO
                Dont hardcode the aspect mask
        */
        sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        image_barrier.subresourceRange = sub_range;
        image_barrier.image = image;

        VkDependencyInfo dep_info = {};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;

        vkCmdPipelineBarrier2(buffer, &dep_info);
}

/* This is to setup to render. Move data onto gpu, etc. Only call this once. */
void heph_renderer_setup_render(HephRenderer *const r)
{
        /*
                todo
                proabably delete this function
        */
}

static void command_buffer_recorder_thread_start_routine()
{
}

/* This is to actually render. Call this in a loop. */
void heph_renderer_render_frame(HephRenderer *const r)
{

        /*
                TODO
                figure out if aborting here is the right call

                maybe return early and dont render the frame?
        */

        if (vkWaitForFences(r->ldevice, 1, &r->render_complete_fence, VK_TRUE, 500000000) != VK_SUCCESS)
        {
                return;
                HEPH_ABORT("I DONT KNOW WHAAT TO PUT HERRE TBH.");
        }
        if (vkResetFences(r->ldevice, 1, &r->render_complete_fence) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to reset render fence. This will mess up rendering.");
        }

        // get next image
        uint32_t image_index;
        if (vkAcquireNextImageKHR(r->ldevice, r->swapchain, 500000000, r->next_image_grabbed_semaphore, NULL, &image_index) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to get image to draw to.");
        }

        /*

                TODO
                un-crap this

        */

        struct HephCommandBufferRecordingInfo
        {
                VkCommandBuffer command_buffer;
                VkImage image;
                void (*commands)(VkCommandBuffer);
        };

        auto commands = [](HephCommandBufferRecordingInfo *ri)
        {
                VkCommandBufferBeginInfo begin_info = {};
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                if (vkBeginCommandBuffer(ri->command_buffer, &begin_info) != VK_SUCCESS)
                {
                        HEPH_ABORT("Failed to begin command buffer recording");
                }

                translate_image_layout(ri->command_buffer, ri->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

                // VkImageMemoryBarrier2 image_barrier = {};
                // image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                /*
                        TODO:
                        its only a good idea to use all commands bit when youre ***NOT*** transitioning an image
                */
                // image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                // image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
                // image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                // image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

                // image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                // image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;

                /*
                        TODO
                        i dont know if these are correct;
                */
                VkImageSubresourceRange range = {};
                range.baseMipLevel = 0;
                range.levelCount = 1;
                range.baseArrayLayer = 0;
                range.layerCount = 1;
                range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                VkClearColorValue clear_color = {{1.0f, 0.0f, 0.0f, 1.0f}};

                vkCmdClearColorImage(ri->command_buffer, ri->image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

                translate_image_layout(ri->command_buffer, ri->image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

                vkEndCommandBuffer(ri->command_buffer);
        }

        HephCommandBufferRecordingInfo recording_info;
        recording_info.buffer = r->frame_render_infos[image_index].command_buffer;

        pthread_t thread;
        if (pthread_create(&thread, NULL, &H, NULL) != 0)
        {
        }

        /*
                todo
                extract all this crap into functions later



        */
        // auto commands = [](VkCommandBuffer buffer, VkQueue queue)
        // {

        // };
        // record_command_buffer(r->command_buffer, r->queue, commands);
        // enqueue_command_buffer(r->command_buffer, r->queue, VK_NULL_HANDLE);

        VkCommandBufferSubmitInfoKHR command_info = {};
        command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_info.commandBuffer = r->command_buffer;

        VkSubmitInfo2 submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.pCommandBufferInfos = &command_info;

        if (vkQueueSubmit2(r->queue, 1, &submit_info, r->render_complete_fence) != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to submit command buffer to render frame.")
        }

        /*

                TODO
                this result thing is wack
        */
        VkResult present_result = VK_NOT_READY;

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &r->swapchain;
        present_info.pImageIndices = &image_index;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &r->render_complete_semaphore;

        if (vkQueuePresentKHR(r->queue, &present_info) != VK_SUCCESS || present_result != VK_SUCCESS)
        {
                HEPH_ABORT("Failed to present frame.");
        }
}

void heph_renderer_destroy(HephRenderer *const r)
{
        vkDeviceWaitIdle(r->ldevice);

        glfwDestroyWindow(r->window);
        glfwTerminate();

        /* Do not change ordering */
        for (uint32_t i = 0; i < r->swapchain_image_count; i++)
        {
                vkDestroyImage(r->ldevice, r->swapchain_images[i], NULL);
        }
        free(r->swapchain_images);
        vkDestroyCommandPool(r->ldevice, r->command_pool, NULL);
        vkFreeMemory(r->ldevice, r->data_buffer_memory, NULL);
        vkDestroyBuffer(r->ldevice, r->data_buffer, NULL);
        vkb::destroy_swapchain(r->vkb_swapchain);
        vkb::destroy_surface(r->vkb_instance, r->surface);
        vkb::destroy_device(r->vkb_ldevice);
        vkb::destroy_instance(r->vkb_instance);
}
