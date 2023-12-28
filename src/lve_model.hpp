#pragma once

#include "glm/fwd.hpp"
#include "lve_device.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <vector>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace lve {

	class LveModel {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal{};
		glm::vec2 uv{};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex &other) const {
			return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
		}
	};

	struct Builder {
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		void loadModel(const std::string &filepath);
	};

	LveModel(LveDevice &device, const Builder& builder);
	~LveModel();

	LveModel(const LveModel &) = delete;
	LveModel &operator=(const LveModel &) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

	static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath);

private:

	void createVertexBuffers(const std::vector<Vertex> &vertices);
	void createIndexBuffers(const std::vector<uint32_t> &indices);

	LveDevice& lveDevice;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	uint32_t vertexCount;


	bool hasIndexBuffer = false;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	uint32_t indexCount;

};

}
