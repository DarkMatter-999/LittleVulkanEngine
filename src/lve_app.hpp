#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_model.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <vector>

namespace lve {

class LveApp {
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	void run();

	LveApp();
	~LveApp();

	LveApp(const LveApp&) = delete;
	LveApp &operator=(const LveApp&) = delete;

private:
	void loadModels();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void recordCommandBuffer(int imageIndex);
	void freeCommandBuffers();
	

	LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
	LveDevice lveDevice{lveWindow};
	std::unique_ptr<LveSwapChain> lveSwapChain;
	std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<LveModel> lveModel;
	// LvePipeline lvePipeline{lveDevice, "shaders/simple.vert.spv", "shaders/simple.frag.spv", LvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
};

}
