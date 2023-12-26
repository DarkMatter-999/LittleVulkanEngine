#include "lve_app.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace lve {

struct SimplePushConstantData {
	glm::mat2 transform{1.0f};
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

LveApp::LveApp() {
	loadGameObjects();
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
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
	clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
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

	renderGameObjects(commandBuffers[imageIndex]);
	
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

void LveApp::loadGameObjects() {
	std::vector<LveModel::Vertex> vertices{
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};
	auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

	std::vector<glm::vec3> colors{
		{1.f, .7f, .73f},
		{1.f, .87f, .73f},
		{1.f, 1.f, .73f},
		{.73f, 1.f, .8f},
		{.73, .88f, 1.f}
	};

	// auto triangle = LveGameObject::createGameObject();
	// triangle.model = lveModel;
	// triangle.color = {0.1f, 0.8f, 0.1f};
	// triangle.transform2d.scale = {2.f, .5f};
	// triangle.transform2d.translation.x = .2f;
	// triangle.transform2d.rotation = .25f * glm::two_pi<float>();
	//
	// gameObjects.push_back(std::move(triangle));
	
	for (int i = 0; i < 40; i++) {
		auto triangle = LveGameObject::createGameObject();
	    triangle.model = lveModel;
		triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
	    triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
		triangle.color = colors[i % colors.size()];
		gameObjects.push_back(std::move(triangle));
	}
}

void LveApp::renderGameObjects(VkCommandBuffer commandBuffer) {
	int i = 0;
  for (auto& obj : gameObjects) {
    i += 1;
    obj.transform2d.rotation =
        glm::mod<float>(obj.transform2d.rotation + 0.001f * i, 2.f * glm::pi<float>());
  }


	lvePipeline->bind(commandBuffer);
	for (auto& obj : gameObjects) {
		obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

		SimplePushConstantData push{};
		push.offset = obj.transform2d.translation;
		push.color = obj.color;
		push.transform = obj.transform2d.mat2();

		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		
		obj.model->bind(commandBuffer);
		obj.model->draw(commandBuffer);
	}
}
}
