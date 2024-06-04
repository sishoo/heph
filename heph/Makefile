
COMPILER = clang++
ENTRY_POINT = main.cpp
BINARY_NAME = main

ALL = $(ENTRY_POINT) -o $(BINARY_NAME) -Wall

VULKAN_INCLUDE_PATH = /usr/local/include
GLFW_INCLUDE_PATH = /usr/local/Cellar/glfw/3.4/include/GLFW
SHADERC_INCLUDE_PATH = /usr/local/Cellar/shaderc/2024.0/include
HEPH_INCLUDE_PATH = /Users/macfarrell/engine/heph
INCLUDE = -I$(VULKAN_INCLUDE_PATH) -I$(GLFW_INCLUDE_PATH) -I$(SHADERC_INCLUDE_PATH) -I$(HEPH_INCLUDE_PATH)

LIBVULKAN_PATH = /Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib
LIBGLFW_PATH = /usr/local/Cellar/glfw/3.4/lib
LIBSHADERC_PATH = /usr/local/Cellar/shaderc/2024.0/lib
LINK = -L$(LIBVULKAN_PATH) -L$(LIBGLFW_PATH) -L$(LIBSHADERC_PATH) -lvulkan -lglfw -lshaderc

debug: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -O0 -fsanitize=address -ldl 

release: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -Ofast





heph_clean: $(ENTRY_POINT)
	clang-tidy $(ENTRY_POINT) -- $(INCLUDE)