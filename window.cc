#include "window.hh"

#include <stdexcept>
#include <sstream>
#include <iostream>

#include <SDL_vulkan.h>

int Window::window_instances = 0;

Window::Window(const char* window_name, int height, int width) {
	if (window_instances == 0) {
		if (SDL_Init(SDL_INIT_VIDEO) != 0)  {
			throw std::runtime_error("Couldn't initialize SDL");
		}
	}
	window_instances++;

	SDL_Window* window = SDL_CreateWindow(
		window_name,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		0,
		0,
		SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN_DESKTOP
	);

	if (window == NULL) {
		throw std::runtime_error("Couldn't create window");
	}
}

void Window::getInstanceExtensions(unsigned int* extension_count, const char** extension_names) {
	if (!SDL_Vulkan_GetInstanceExtensions(
			window,
			extension_count,
			extension_names)
	) {
		std::string error_msg = "Couldn't get vulkan instance extensions :(\nSDL says: ";
		error_msg.append(SDL_GetError());
		throw std::runtime_error(error_msg);
	};
}

Window::~Window() {
	SDL_DestroyWindow(window);
	if (--window_instances == 0) {
		SDL_Quit();
	}

}
