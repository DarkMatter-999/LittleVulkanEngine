#include "lve_app.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_render_system.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <array>

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

	while (!lveWindow.shouldClose()) {
		glfwPollEvents();
		if (auto commandBuffer = lveRenderer.beginFrame()) {
			lveRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
			lveRenderer.endSwapChainRenderPass(commandBuffer);
			lveRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(lveDevice.device());
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
}
