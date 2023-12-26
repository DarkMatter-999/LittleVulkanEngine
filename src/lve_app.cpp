#include "lve_app.hpp"
#include "GLFW/glfw3.h"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <stdexcept>
#include <array>

namespace lve {

LveApp::LveApp() {
	loadModels();
	createPipelineLayout();
	recreateSwapChain();
	createCommandBuffers();
}

LveApp::~LveApp() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

void LveApp::run() {
	while (!lveWindow.shouldClose()) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(lveDevice.device());
}

void LveApp::createPipelineLayout() {
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

void LveApp::createPipeline() {
	assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipeLineConfigInfo pipelineConfig{};
	LvePipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = lveSwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<LvePipeline>(lveDevice,"shaders/simple.vert.spv","shaders/simple.frag.spv",pipelineConfig);
}

void LveApp::createCommandBuffers() {
	commandBuffers.resize(lveSwapChain->imageCount());

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = lveDevice.getCommandPool();
	allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if(vkAllocateCommandBuffers(lveDevice.device(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers");
	}

}

void LveApp::recreateSwapChain() {
	auto extent = lveWindow.getExtent();
	while(extent.width == 0 || extent.height == 0) {
		extent = lveWindow.getExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(lveDevice.device());
	
	if (lveSwapChain == nullptr) {
		lveSwapChain = std::make_unique<LveSwapChain>(lveDevice,extent);
	} else {
		lveSwapChain = std::make_unique<LveSwapChain>(lveDevice,extent, std::move(lveSwapChain));
		if (lveSwapChain->imageCount() != commandBuffers.size()) {
			freeCommandBuffers();
			createCommandBuffers();
		}
	}
	createPipeline();
}

void LveApp::recordCommandBuffer(int imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = lveSwapChain->getRenderPass();
	renderpassBeginInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

	renderpassBeginInfo.renderArea.offset = {0,0};
	renderpassBeginInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
	clearValues[1].depthStencil = {1.0f, 0};

	renderpassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderpassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
	vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);


	lvePipeline->bind(commandBuffers[imageIndex]);

	lveModel->bind(commandBuffers[imageIndex]);
	lveModel->draw(commandBuffers[imageIndex]);

	vkCmdEndRenderPass(commandBuffers[imageIndex]);

	if(vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
	throw std::runtime_error("Failed to record command buffer");
	}
}

void LveApp::freeCommandBuffers() {
	vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
}

void LveApp::drawFrame() {
	uint32_t imageIndex;
	auto result = lveSwapChain->acquireNextImage(&imageIndex);

	if(result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	recordCommandBuffer(imageIndex);

	result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
		lveWindow.resetWindowResizedFlag();
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void LveApp::loadModels() {
	std::vector<LveModel::Vertex> vertices{
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};
	lveModel = std::make_unique<LveModel>(lveDevice, vertices);
}

}
