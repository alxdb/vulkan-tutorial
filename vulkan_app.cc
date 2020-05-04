#include "vulkan_app.hh"

bool VulkanApp::poll_events() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT:
				return false;
		}
	}
	return true;
}

void VulkanApp::run() {
	while (poll_events()) {
		render();
	}
}

VulkanApp::~VulkanApp() {
	device->waitIdle();
};

vk::UniqueInstance create_instance(const Window& window, const char* application_name, bool debug);

uint32_t find_graphics_queue(
	const vk::PhysicalDevice& physical_device,
	const vk::UniqueInstance& instance,
	const vk::UniqueSurfaceKHR& surface
);

vk::UniqueDevice create_logical_device(
	const vk::PhysicalDevice& physical_device,
	uint32_t graphics_queue_family
);


VulkanApp::VulkanApp(const char* application_name, bool debug) : window(application_name, 0, 0) {
	instance = create_instance(window, application_name, debug);
	surface = window.create_surface(instance);
	physical_device = instance->enumeratePhysicalDevices().front();
	graphics_queue_family = find_graphics_queue(physical_device, instance, surface);
	device = create_logical_device(physical_device, graphics_queue_family);
	graphics_queue = device->getQueue(graphics_queue_family, 0);
	create_swapchain();
}

vk::UniqueInstance create_instance(const Window& window, const char* application_name, bool debug) {
	std::vector<const char *> extension_names = window.get_instance_extensions();
	std::vector<const char *> layer_names;
	if (debug) {
		layer_names.push_back("VK_LAYER_KHRONOS_validation");
	}

	vk::ApplicationInfo application_info({}, {}, application_name);
	vk::InstanceCreateInfo instance_create_info(
		{},
		&application_info,
		(uint32_t) layer_names.size(),
		layer_names.data(),
		(uint32_t) extension_names.size(),
		extension_names.data()
	);
	return vk::createInstanceUnique(instance_create_info);
}

uint32_t find_graphics_queue(
	const vk::PhysicalDevice& physical_device,
	const vk::UniqueInstance& instance,
	const vk::UniqueSurfaceKHR& surface
) {
	auto queue_family_properties = physical_device.getQueueFamilyProperties();
	for (int i = 0; i < queue_family_properties.size(); i++) {
		if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
			physical_device.getSurfaceSupportKHR(i, surface.get())) {
			return i;
		}
	};
	throw std::runtime_error("device is incompatible");
}

vk::UniqueDevice create_logical_device(
	const vk::PhysicalDevice& physical_device,
	uint32_t graphics_queue_family
) {
	float queue_priority = 1.0f;
	vk::DeviceQueueCreateInfo queue_create_info({}, graphics_queue_family, 1, &queue_priority);
	vk::DeviceCreateInfo device_create_info(
		{},
		1,
		&queue_create_info
	);
	device_create_info.setEnabledExtensionCount(1);
	const char* extension_names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	device_create_info.setPpEnabledExtensionNames(extension_names);
	return physical_device.createDeviceUnique(device_create_info);
}
