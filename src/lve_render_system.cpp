#include "lve_render_system.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#include "lve_frame_info.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_renderer.hpp"
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
	glm::mat4 transform{1.0f};
	glm::mat4 normalMatrix{1.0f};
};

LveRenderSystem::LveRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{device} {
  createPipelineLayout();
  createPipeline(renderPass);
}

LveRenderSystem::~LveRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

void LveRenderSystem::createPipelineLayout() {
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
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void LveRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipeLineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "shaders/simple.vert.spv",
      "shaders/simple.frag.spv",
      pipelineConfig);
}

void LveRenderSystem::renderGameObjects(FrameInfo &frameInfo, std::vector<LveGameObject>& gameObjects, const LveCamera &camera) {
	lvePipeline->bind(frameInfo.commandBuffer);

	auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

	for (auto& obj : gameObjects) {
		SimplePushConstantData push{};
		auto modelMatrix = obj.transform.mat4();
		push.transform = projectionView * modelMatrix;
		push.normalMatrix = obj.transform.normalMatrix();

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		
		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}
}
