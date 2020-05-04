#include "window.hh"

#include <stdexcept>
#include <sstream>
#include <iostream>

#include <SDL_vulkan.h>

int Window::window_instances = 0;

Window::Window(const char* window_name, int height, int width) {
	if (window_instances == 0) {
		if (SDL_Init(SDL_INIT_VIDEO) != 0)  {
			throw std::runtime_error("Couldn't initialize SDL :(");
		}
	}
	window_instances++;

	window = SDL_CreateWindow(
		window_name,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		height,
		width,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
	);

	if (window == nullptr) {
		throw std::runtime_error("Couldn't create window :(");
	}
}

std::vector<const char*> Window::get_instance_extensions() const {
	unsigned int extension_count;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr)) {
		std::string error_msg = "Couldn't get vulkan instance extensions :(\nSDL says: ";
		error_msg.append(SDL_GetError());
		throw std::runtime_error(error_msg);
	};
	std::vector<const char*> extensions(extension_count);
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions.data())) {
		std::string error_msg = "Couldn't get vulkan instance extensions :(\nSDL says: ";
		error_msg.append(SDL_GetError());
		throw std::runtime_error(error_msg);
	};
	return extensions;
}

vk::UniqueSurfaceKHR Window::create_surface(vk::UniqueInstance& instance) {
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, instance.get(), &surface)) {
		std::string error_msg = "Couldn't create vulkan surface :(\nSDL says: ";
		error_msg.append(SDL_GetError());
		throw std::runtime_error(error_msg);
	}
	return vk::UniqueSurfaceKHR(surface, instance.get());
}

Window::~Window() {
	SDL_DestroyWindow(window);
	if (--window_instances == 0) {
		SDL_Quit();
	}

}
