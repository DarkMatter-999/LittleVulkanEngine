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

std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
	LveModel::Builder modelBuilder{};
	modelBuilder.vertices = {
	      // left face (white)
	      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
	      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
	      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
	      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

	      // right face (yellow)
	      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
	      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
	      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
	      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

	      // top face (orange, remember y axis points down)
	      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
	      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

	      // bottom face (red)
	      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
	      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
	      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
	      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

	      // nose face (blue)
	      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
	      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

	      // tail face (green)
	      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	  };
	for (auto& v : modelBuilder.vertices) {
	    v.position += offset;
	}

	modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
				  12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

	return std::make_unique<LveModel>(device, modelBuilder);
}



void LveApp::loadGameObjects() {
	std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, {.0f, .0f, .0f});
	auto cube = LveGameObject::createGameObject();
	cube.model = lveModel;
	cube.transform.translation = {.0f, .0f, 4.5f};
	cube.transform.scale = {.5f, .5f, .5f};

	gameObjects.push_back(std::move(cube));
}
}
