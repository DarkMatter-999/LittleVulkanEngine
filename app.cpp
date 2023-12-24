#include "app.hpp"

namespace lve {

void FirstApp::run() {
	while (!lveWindow.shouldClose()) {
		glfwPollEvents();
	}
}
}
