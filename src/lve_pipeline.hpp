#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "lve_device.hpp"
#include "vulkan/vulkan_core.h"

namespace lve {

struct PipeLineConfigInfo {

	PipeLineConfigInfo() = default;
	PipeLineConfigInfo(const PipeLineConfigInfo&) = delete;
	PipeLineConfigInfo& operator=(const PipeLineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class LvePipeline {
public:
	LvePipeline(LveDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipeLineConfigInfo& config);

	~LvePipeline();

	LvePipeline(const LvePipeline&) = delete;
	LvePipeline& operator=(const LvePipeline&) = delete;

	static void defaultPipelineConfigInfo(PipeLineConfigInfo& configInfo);
	static void enableAlphaBlending(PipeLineConfigInfo& configInfo);

	void bind(VkCommandBuffer commandBuffer);


private:
	static std::vector<char> readFile(const std::string& filePath);
	
	void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipeLineConfigInfo& config);

	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	LveDevice& lveDevice;
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};

}
