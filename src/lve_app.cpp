#include "lve_app.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#include "lve_camera.hpp"
#include "lve_frame_info.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_buffer.hpp"
#include "lve_render_system.hpp"
#include "lve_input.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace lve {

struct GlobalUbo {
	glm::mat4 projectionView{1.0f};
	glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
	glm::vec3 lightPosition{-1.0f};
	alignas(16) glm::vec4 lightColor{1.0f};
};

LveApp::LveApp() {
	globalPool =
      LveDescriptorPool::Builder(lveDevice).setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT).build();
	loadGameObjects();
}

LveApp::~LveApp() { }

void LveApp::run() {
std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++) {
		uboBuffers[i] = std::make_unique<LveBuffer>(
			lveDevice,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		uboBuffers[i]->map();
	}

	
	auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build();

	std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		LveDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
	}

	LveRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
	LveCamera camera{};

	auto viewerObject = LveGameObject::createGameObject();
	LveKeyboardMovementController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();

	while (!lveWindow.shouldClose()) {
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
		
		float aspect = lveRenderer.getAspectRatio();
		// camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
		camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
		
		if (auto commandBuffer = lveRenderer.beginFrame()) {
			int frameIndex = lveRenderer.getFrameIndex();

			FrameInfo frameInfo {
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex]
			};

			// Update
			GlobalUbo ubo{};
			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			// Render
			lveRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(frameInfo, gameObjects, camera);
			lveRenderer.endSwapChainRenderPass(commandBuffer);
			lveRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(lveDevice.device());
}


void LveApp::loadGameObjects() {
	std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
	auto flatVase = LveGameObject::createGameObject();
	flatVase.model = lveModel;
	flatVase.transform.translation = {-.5f, .5f, 2.5f};
	flatVase.transform.translation = {-.5f, .5f, 0.f};
	flatVase.transform.scale = {3.f, 1.5f, 3.f};
	gameObjects.push_back(std::move(flatVase));

	lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
	auto smoothVase = LveGameObject::createGameObject();
	smoothVase.model = lveModel;
	smoothVase.transform.translation = {.5f, .5f, 2.5f};
	smoothVase.transform.translation = {.5f, .5f, 0.f};
	smoothVase.transform.scale = {3.f, 1.5f, 3.f};
	gameObjects.push_back(std::move(smoothVase));

	lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
	auto floor = LveGameObject::createGameObject();
	floor.model = lveModel;
	floor.transform.translation = {0.f, .5f, 0.f};
	floor.transform.scale = {3.f, 1.f, 3.f};
	gameObjects.push_back(std::move(floor));
}
}
