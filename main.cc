#include <iostream>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <filesystem>

#include "window.hh"
#include "vulkan_app.hh"

int main() {
	VulkanApp app("vulkan_tutorial");
	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
