#pragma once

#include "glm/fwd.hpp"
#include "lve_frame_info.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_game_object.hpp"
#include "lve_camera.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <vector>

namespace lve {

class LveRenderSystem {
public:
	LveRenderSystem(LveDevice& device, VkRenderPass renderPass);
	~LveRenderSystem();

	LveRenderSystem(const LveRenderSystem&) = delete;
	LveRenderSystem &operator=(const LveRenderSystem&) = delete;

	void renderGameObjects(FrameInfo& frameInfo, std::vector<LveGameObject>& gameObjects, const LveCamera &camera);
private:
	void createPipelineLayout();
	void createPipeline(VkRenderPass renderPass);
	
	LveDevice& lveDevice;

	std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
};

}
