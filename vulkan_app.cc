#include "vulkan_app.hh"
#include "vulkan_utils.hh"

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

void VulkanApp::render() {
	device->waitForFences({in_flight_fences[current_frame].get()}, true, UINT64_MAX);
	uint32_t image_index = device->acquireNextImageKHR(
		swapchain.get(),
		UINT64_MAX,
		image_available_semaphores[current_frame].get(),
		nullptr
	);
	if (images_in_flight[image_index] != (vk::Fence) nullptr) {
		device->waitForFences({images_in_flight[image_index]}, true, UINT64_MAX);
	}
	images_in_flight[image_index] = in_flight_fences[current_frame].get();
	device->resetFences({in_flight_fences[current_frame].get()});

	vk::Semaphore wait_semaphores[] = { image_available_semaphores[current_frame].get() };
	vk::Semaphore signal_semaphores[] = { render_finished_semaphores[current_frame].get() };
	vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::SubmitInfo submit_info(
		1, wait_semaphores,
		wait_stages,
		1, &command_buffers[image_index].get(),
		1, signal_semaphores
	);
	graphics_queue.submit({submit_info}, in_flight_fences[current_frame].get());

	vk::SwapchainKHR swapchains[] = { swapchain.get() };
	vk::PresentInfoKHR present_info(
		1, signal_semaphores,
		1, swapchains,
		&image_index
	);
	graphics_queue.presentKHR(present_info);

	current_frame = (current_frame + 1) % max_frames_in_flight;
}

VulkanApp::VulkanApp(const char* application_name, bool debug) : window(application_name, 0, 0) {
	instance = create_instance(window, application_name, debug);
	surface = window.create_surface(instance);

	physical_device = instance->enumeratePhysicalDevices().front();
	uint32_t graphics_queue_family = find_graphics_queue(physical_device, instance, surface);
	device = create_logical_device(physical_device, graphics_queue_family);
	graphics_queue = device->getQueue(graphics_queue_family, 0);

	command_pool = device->createCommandPoolUnique({{}, graphics_queue_family});

	pipeline_layout = device->createPipelineLayoutUnique({});
	vert_shader = create_shader(device, "build/main.vert.spv");
	frag_shader = create_shader(device, "build/main.frag.spv");

	generate_swapchain();

	// Sync Objects
	image_available_semaphores.reserve(max_frames_in_flight);
	render_finished_semaphores.reserve(max_frames_in_flight);
	in_flight_fences.reserve(max_frames_in_flight);
	for (int i = 0; i < max_frames_in_flight; i++) {
		image_available_semaphores.push_back(device->createSemaphoreUnique({}));
		render_finished_semaphores.push_back(device->createSemaphoreUnique({}));
		in_flight_fences.push_back(device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled}));
	}
	images_in_flight.resize(swapchain_image_views.size(), nullptr);
}

void VulkanApp::generate_swapchain() {
	device->waitIdle();

	vk::SurfaceCapabilitiesKHR surface_capabilities =
		physical_device.getSurfaceCapabilitiesKHR(surface.get());
	vk::SurfaceFormatKHR surface_format =
		physical_device.getSurfaceFormatsKHR(surface.get()).front();

	swapchain = create_swapchain(device, surface, surface_capabilities, surface_format);
	swapchain_image_views = create_swapchain_image_views(device, swapchain, surface_format);

	// Graphics Pipeline
	render_pass = create_render_pass(device, surface_format);
	graphics_pipeline = create_pipeline(
		device,
		surface_capabilities,
		vert_shader,
		frag_shader,
		pipeline_layout,
		render_pass
	);
	swapchain_frame_buffers = create_frame_buffers(
		device,
		render_pass,
		surface_capabilities,
		swapchain_image_views
	);
	command_buffers = create_command_buffers(
		device,
		command_pool,
		render_pass,
		graphics_pipeline,
		surface_capabilities,
		swapchain_frame_buffers
	);
};
