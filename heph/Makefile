
COMPILER = clang++

ENTRY_POINT = main.cpp

BINARY_NAME = heph

ALL = $(ENTRY_POINT) -o $(BINARY_NAME) -Wall

VULKAN_INCLUDE_PATH = /Users/macfarrell/VulkanSDK/1.3.261.1/macOS/include
GLFW_INCLUDE_PATH = /usr/local/Cellar/glfw/3.4/include
LIBSHADERC_PATH = /usr/local/Cellar/shaderc/2024.0/include
INCLUDE = -I$(VULKAN_INCLUDE_PATH) -I$(GLFW_INCLUDE_PATH) -I$(LIBSHADERC_PATH)

VULKAN_LIB_PATH = /Users/macfarrell/VulkanSDK/1.3.261.1/macOS/lib
GLFW_LIB_PATH = /usr/local/Cellar/glfw/3.4/lib
LIBSHADERC_LIB_PATH = /usr/local/Cellar/shaderc/2024.0/lib
LINK = -l$(VULKAN_LIB_PATH) -l$(GLFW_LIB_PATH) -I$(LIBSHADERC_LIB_PATH)

heph_debug: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -O0 -fsanitize=address -ldl

heph_release: $(ENTRY_POINT)
	$(COMPILER) $(ALL) $(LINK) $(INCLUDE) -Ofast




