BUILD_DIR=./build

CXX = g++
CXXFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib -lglfw3 -lvulkan-1

SRC_FILES = $(wildcard *.cpp)
OBJ_FILES = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SRC_FILES)))
SHADER_FILES = $(wildcard shaders/*.vert shaders/*.frag)
SPV_FILES = $(patsubst shaders/%.vert,$(BUILD_DIR)/shaders/%.vert.spv,$(SHADER_FILES)) \
            $(patsubst shaders/%.frag,$(BUILD_DIR)/shaders/%.frag.spv,$(SHADER_FILES))

lve: $(OBJ_FILES) 
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/lve $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/shaders/%.vert.spv: shaders/%.vert
	mkdir -p $(BUILD_DIR)/shaders
	$(VULKAN_SDK_PATH)/bin/glslc $< -o $@

$(BUILD_DIR)/shaders/%.frag.spv: shaders/%.frag
	mkdir -p $(BUILD_DIR)/shaders
	$(VULKAN_SDK_PATH)/bin/glslc $< -o $@

.PHONY: test clean shader run

shader: $(SPV_FILES)

run:
	$(BUILD_DIR)/lve

clean:
	rm -rf $(BUILD_DIR)

