
COMPILER = clang++

ENTRY_POINT = main.cpp

BINARY_NAME = heph

ALL = $(ENTRY_POINT) -o $(BINARY_NAME) -Wall

VULKAN_INCLUDE_PATH = /usr/local/include/vulkan
GLFW_INCLUDE_PATH = /usr/local/Cellar/glfw/3.4/include/GLFW
LIBSHADERC_PATH = /usr/local/include/shaderc
INCLUDE = -I$(VULKAN_INCLUDE_PATH) -I$(GLFW_INCLUDE_PATH) -I$(LIBSHADERC_PATH)

VULKAN_LIB_PATH = /Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib
GLFW_LIB_PATH = /usr/local/Cellar/glfw/3.4/lib
LIBSHADERC_LIB_PATH = /usr/local/Cellar/shaderc/2024.0/lib
LINK = -L$(VULKAN_LIB_PATH) -L$(GLFW_LIB_PATH) -L$(LIBSHADERC_LIB_PATH) -lvulkan -lglfw -lshaderc

debug: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -O0 -fsanitize=address -ldl -I./include

release: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -Ofast





heph_clean: $(ENTRY_POINT)
	clang-tidy $(ENTRY_POINT) -- $(INCLUDE)