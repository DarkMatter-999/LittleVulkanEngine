BUILD_DIR=./build

CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include 
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib -lglfw3 -lvulkan-1

lve: *.cpp *.hpp
	mkdir -p $(BUILD_DIR)
	g++ $(CFLAGS) -o $(BUILD_DIR)/lve *.cpp $(LDFLAGS)

.PHONY: test clean

shader:
	$(VULKAN_SDK_PATH)/bin/glslc shaders/simple.vert -o shaders/simple.vert.spv
	$(VULKAN_SDK_PATH)/bin/glslc shaders/simple.frag -o shaders/simple.frag.spv

run:
	$(BUILD_DIR)/lve
