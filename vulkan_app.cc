#include "vulkan_app.hh"
#include "vulkan_utils.hh"
#include "main.frag.spv.h"
#include "main.vert.spv.h"

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
		update();
		render();
	}
}

VulkanApp::VulkanApp(
	const char* application_name,
	std::vector<Vertex> init_vertices,
	bool debug
) : window(application_name, 1024, 768),
	vertices(init_vertices),
	instance(create_instance(window, application_name, debug)),
	surface(window.create_surface(instance)),
	physical_device(instance->enumeratePhysicalDevices().front()) ,
	graphics_queue_family(find_graphics_queue(physical_device, instance, surface)),
	device(create_logical_device(physical_device, graphics_queue_family)),
	graphics_queue(device->getQueue(graphics_queue_family, 0)),
	vert_shader(device->createShaderModuleUnique({{}, main_vert_spv_len, reinterpret_cast<const uint32_t *>(main_vert_spv)})),
	frag_shader(device->createShaderModuleUnique({{}, main_frag_spv_len, reinterpret_cast<const uint32_t *>(main_frag_spv)})),
	pipeline_layout(device->createPipelineLayoutUnique({})),
	command_pool(device->createCommandPoolUnique({{}, graphics_queue_family}))
{
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

VulkanApp::~VulkanApp() {
	device->waitIdle();
};

void VulkanApp::render() {
	device->waitForFences({in_flight_fences[current_frame].get()}, true, UINT64_MAX);

	uint32_t image_index;
	vk::Result aquire_image_result = device->acquireNextImageKHR(
		swapchain.get(),
		UINT64_MAX,
		image_available_semaphores[current_frame].get(),
		nullptr,
		&image_index
	);

	switch (aquire_image_result) {
		case vk::Result::eErrorOutOfDateKHR:
			generate_swapchain();
			return;
		default:
			if (aquire_image_result != vk::Result::eSuccess &&
				aquire_image_result != vk::Result::eSuboptimalKHR) {
				vk::throwResultException(aquire_image_result, "");
			}
	}

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
		render_pass,
		Vertex::binding_description,
		Vertex::attribute_descriptions
	);
	swapchain_frame_buffers = create_frame_buffers(
		device,
		render_pass,
		surface_capabilities,
		swapchain_image_views
	);
	std::tie(vertex_buffer, vertex_memory) = create_vertex_buffer(
		physical_device,
		device,
		vertices
	);
	command_buffers = create_command_buffers(
		device,
		command_pool,
		render_pass,
		graphics_pipeline,
		surface_capabilities,
		swapchain_frame_buffers,
		vertex_buffer,
		vertices.size()
	);
};
