#include <iostream>
#include <memory>
#include <stdexcept>

#include <vulkan/vulkan.hpp>
#include <SDL.h>

#include "window.hh"

class App {
	Window window;
	vk::UniqueInstance instance;
	vk::PhysicalDevice physical_device;
	uint32_t graphics_queue_family;
	vk::UniqueDevice device;
	vk::Queue graphics_queue;
	vk::UniqueSurfaceKHR surface;

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
	explicit App(const char * application_name) : window(application_name, 0, 0) {
		{ // Instance Creation
			std::vector<const char *> extension_names = window.getInstanceExtensions();
			std::vector<const char *> layer_names = {
#ifndef NDEBUG
				"VK_LAYER_KHRONOS_validation",
#endif
			};

			vk::ApplicationInfo application_info{{}, {}, application_name};
			instance = vk::createInstanceUnique({
				{},
				&application_info,
				(uint32_t) layer_names.size(),
				layer_names.data(),
				(uint32_t) extension_names.size(),
				extension_names.data()
			});
		}
		{ // Window Surface
			surface = window.createSurface(instance);
		}
		{ // Physical Device
			physical_device = instance->enumeratePhysicalDevices().front();

			auto queue_family_properties = physical_device.getQueueFamilyProperties();
			bool queue_family_found = false;
			for (auto it = queue_family_properties.begin(); it < queue_family_properties.end() && !queue_family_found; it++) {
				if ((*it).queueFlags & vk::QueueFlagBits::eGraphics) {
					graphics_queue_family = std::distance(queue_family_properties.begin(), it);
					if (!physical_device.getSurfaceSupportKHR(graphics_queue_family, surface.get())) {
						throw std::runtime_error("device is incompatible");
					}
					queue_family_found = true;
				}
			};
			if (!queue_family_found) {
				throw std::runtime_error("device is incompatible");
			}
		}
		{ // Logical Device
			float queue_priority = 1.0f;
			vk::DeviceQueueCreateInfo queue_create_info({}, graphics_queue_family, 1, &queue_priority);
			device = physical_device.createDeviceUnique({{}, 1, &queue_create_info});
			graphics_queue = device->getQueue(graphics_queue_family, 0);
		}
	}
};

int main() {
	App app("vulkan_tutorial");
	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
