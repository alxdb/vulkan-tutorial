#include <iostream>
#include <memory>
#include <stdexcept>

#include <vulkan/vulkan.hpp>
#include <SDL.h>

#include "window.hh"

class App {
	Window window;
	vk::UniqueInstance instance;
	bool poll_events() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					return false;
			}
		}
		return true;
	}
	void main_loop() {
		while (poll_events()) {
		}
	}
public:
	void run() {
		main_loop();
	}
	App(Window &&_window, vk::UniqueInstance &&_instance) : window(_window), instance(_instance) {
	}
};

App init_app(const char* application_name) {
	Window window(application_name, 0, 0);
	vk::ApplicationInfo application_info(application_name);

	unsigned int extension_count;
	window.getInstanceExtensions(&extension_count, nullptr);
	std::vector<const char *> extension_names;
	extension_names.resize(extension_count);
	window.getInstanceExtensions(&extension_count, extension_names.data());

	std::vector<const char *> layer_names = {
#ifndef NDEBUG
		"VK_LAYER_KHRONOS_validation",
#endif
	};

	vk::InstanceCreateInfo instance_create_info(
		{},
		&application_info,
		layer_names.size(),
		layer_names.data(),
		extension_names.size(),
		extension_names.data()
	);

	return App(
		std::move(window),
		vk::createInstanceUnique(
			instance_create_info,
			nullptr
		)
	);
}

int main() {
	App app = init_app("vulkan_tutorial");
	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
