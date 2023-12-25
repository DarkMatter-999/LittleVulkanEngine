#include "app.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

namespace lve {

FirstApp::FirstApp() {
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

FirstApp::~FirstApp() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

void FirstApp::run() {
	while (!lveWindow.shouldClose()) {
		glfwPollEvents();
	}
}

void FirstApp::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void FirstApp::createPipeline() {
	auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
	pipelineConfig.renderPass = lveSwapChain.getRenderPass();
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<LvePipeline>(lveDevice,"shaders/simple.vert.spv","shaders/simple.frag.spv",pipelineConfig);
}

void FirstApp::drawFrame() {}
void FirstApp::createCommandBuffers() {}
}
