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

class LvePointLightSystem {
public:
	LvePointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~LvePointLightSystem();

	LvePointLightSystem(const LvePointLightSystem&) = delete;
	LvePointLightSystem &operator=(const LvePointLightSystem&) = delete;

	void render(FrameInfo& frameInfo);
private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);
	
	LveDevice& lveDevice;

	std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
};

}
