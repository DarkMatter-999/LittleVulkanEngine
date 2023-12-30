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
	glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
};

LveApp::LveApp() {
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

	LveRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
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
		camera.setPerspectiveProjection(50.0f, aspect, 0.1f, 10.0f);
		
		if (auto commandBuffer = lveRenderer.beginFrame()) {
			int frameIndex = lveRenderer.getFrameIndex();

			FrameInfo frameInfo {
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
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
	std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/suzzane.obj");
	auto model = LveGameObject::createGameObject();
	model.model = lveModel;
	model.transform.translation = {.0f, .0f, 4.5f};
	model.transform.scale = {.5f, .5f, .5f};

	gameObjects.push_back(std::move(model));
}
}
