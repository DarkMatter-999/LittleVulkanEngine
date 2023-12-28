#include "lve_app.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_render_system.hpp"
#include "lve_input.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <array>
#include <chrono>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace lve {

LveApp::LveApp() {
	loadGameObjects();
}

LveApp::~LveApp() { }

void LveApp::run() {
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
			lveRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
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
