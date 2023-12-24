#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "app.hpp"

int main() {
    lve::FirstApp app{};

    try {
	app.run();
    } catch (const std::exception &e) {
	std::cerr << e.what() << '\n';
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
