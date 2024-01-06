#pragma once

#include "glm/fwd.hpp"
#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <vector>

namespace lve {

class LveApp {
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	void run();

	LveApp();
	~LveApp();

	LveApp(const LveApp&) = delete;
	LveApp &operator=(const LveApp&) = delete;

private:
	void loadGameObjects();

	LveWindow lveWindow{WIDTH, HEIGHT, "LittleVulkanEngine"};
	LveDevice lveDevice{lveWindow};

	LveRenderer lveRenderer{lveWindow, lveDevice};

	// order of declarations matters idk why
	std::unique_ptr<LveDescriptorPool> globalPool{};
	LveGameObject::Map gameObjects;
};

}
