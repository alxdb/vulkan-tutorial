#include <iostream>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <filesystem>

#include <vulkan/vulkan.hpp>
#include <SDL.h>

#include "window.hh"

vk::UniqueShaderModule create_shader(const vk::UniqueDevice& device, const std::filesystem::path& spirv_file) {
	std::ifstream file(spirv_file, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}
	std::vector<char> code(file.tellg());
	file.seekg(0);
	file.read(code.data(), code.size());
	file.close();
	return device->createShaderModuleUnique({
		{},
		code.size(),
		reinterpret_cast<const uint32_t*>(code.data())
	});
}

const int MAX_FRAMES_IN_FLIGHT = 2;

class App {
	Window window;
	// Instance
	vk::UniqueInstance instance;
	// Surface
	vk::UniqueSurfaceKHR surface;
	// Physical Device
	vk::PhysicalDevice physical_device;
	uint32_t graphics_queue_family;
	// Logical Device
	vk::UniqueDevice device;
	vk::Queue graphics_queue;
	// Swapchain
	vk::UniqueSwapchainKHR swapchain;
	std::vector<vk::UniqueImageView> swapchain_image_views;
	// Render Pass
	vk::UniqueRenderPass render_pass;
	// Graphics Pipeline
	vk::UniqueShaderModule vert_shader;
	vk::UniqueShaderModule frag_shader;
	vk::UniquePipelineLayout pipeline_layout;
	vk::UniquePipeline graphics_pipeline;
	// Framebuffers
	std::vector<vk::UniqueFramebuffer> swapchain_frame_buffers;
	// Command Pool
	vk::UniqueCommandPool command_pool;
	// Command Buffers
	std::vector<vk::UniqueCommandBuffer> command_buffers;
	// Sync Objects
	std::vector<vk::UniqueSemaphore> image_available_semaphores;
	std::vector<vk::UniqueSemaphore> render_finished_semaphores;
	std::vector<vk::UniqueFence> in_flight_fences;
	std::vector<vk::Fence> images_in_flight;
	size_t current_frame = 0;

	void render() {
		device->waitForFences({in_flight_fences[current_frame].get()}, true, UINT64_MAX);
		uint32_t image_index = device->acquireNextImageKHR(swapchain.get(), UINT64_MAX, image_available_semaphores[current_frame].get(), nullptr);
		if (images_in_flight[image_index] != (vk::Fence) nullptr) {
			device->waitForFences({images_in_flight[image_index]}, true, UINT64_MAX);
		}
		images_in_flight[image_index] = in_flight_fences[current_frame].get();
		vk::Semaphore wait_semaphores[] = { image_available_semaphores[current_frame].get() };
		vk::Semaphore signal_semaphores[] = { render_finished_semaphores[current_frame].get() };
		vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		device->resetFences({in_flight_fences[current_frame].get()});
		graphics_queue.submit({{
				1,
				wait_semaphores,
				wait_stages,
				1,
				&command_buffers[image_index].get(),
				1,
				signal_semaphores
			}},
			in_flight_fences[current_frame].get()
		);
		vk::SwapchainKHR swapchains[] = { swapchain.get() };
		graphics_queue.presentKHR({
			1,
			signal_semaphores,
			1,
			swapchains,
			&image_index
		});
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

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

public:
	void run() {
		while (poll_events()) {
			render();
		}
		device->waitIdle();
	}
	explicit App(const char * application_name) : window(application_name, 0, 0) {
		{ // Instance Creation
			std::vector<const char *> extension_names = window.get_instance_extensions();
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
			surface = window.create_surface(instance);
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
			std::vector<const char *> extension_names = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
			vk::DeviceQueueCreateInfo queue_create_info({}, graphics_queue_family, 1, &queue_priority);
			device = physical_device.createDeviceUnique({
				{},
				1,
				&queue_create_info,
				0,
				nullptr,
				(uint32_t) extension_names.size(),
				extension_names.data()
			});
			graphics_queue = device->getQueue(graphics_queue_family, 0);
		}
		std::vector<vk::Image> swapchain_images;
		vk::Format swapchain_image_format;
		vk::Extent2D swapchain_extent;
		{ // Swapchain
			vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface.get());
			swapchain_extent = surface_capabilities.currentExtent;
			vk::SurfaceFormatKHR surface_format = physical_device.getSurfaceFormatsKHR(surface.get()).front();
			swapchain_image_format = surface_format.format;
			std::vector<vk::PresentModeKHR> present_modes = physical_device.getSurfacePresentModesKHR(surface.get());
			vk::PresentModeKHR present_mode = *std::find(present_modes.begin(), present_modes.end(), vk::PresentModeKHR::eFifo);
			swapchain = device->createSwapchainKHRUnique({
				{},
				surface.get(),
				surface_capabilities.minImageCount,
				swapchain_image_format,
				surface_format.colorSpace,
				swapchain_extent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				surface_capabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				present_mode,
				true,
				nullptr
			});
			swapchain_images = device->getSwapchainImagesKHR(swapchain.get());
		}
		{ // Image Views
			swapchain_image_views.reserve(swapchain_images.size());
			for (const vk::Image& swapchain_image : swapchain_images) {
				swapchain_image_views.push_back(device->createImageViewUnique({
					{},
					swapchain_image,
					vk::ImageViewType::e2D,
					swapchain_image_format,
					{},
					{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
				}));
			}
		}
		{ // Render Pass
			vk::SubpassDependency dependency(
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlags(),
				vk::AccessFlagBits::eColorAttachmentWrite
			);
			vk::AttachmentDescription color_attachment(
				{},
				swapchain_image_format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			);
			vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
			vk::SubpassDescription subpass(
				{},
				vk::PipelineBindPoint::eGraphics,
				0, nullptr,
				1, &color_attachment_ref);
			render_pass = device->createRenderPassUnique({
				{},
				1,
				&color_attachment,
				1,
				&subpass,
				1,
				&dependency
			});
		}
		{ // Graphics Pipeline
			vert_shader = create_shader(device, "build/main.vert.spv");
			frag_shader = create_shader(device, "build/main.frag.spv");
			vk::PipelineShaderStageCreateInfo shader_stages[] = {
				{{}, vk::ShaderStageFlagBits::eVertex, vert_shader.get(), "main"},
				{{}, vk::ShaderStageFlagBits::eFragment, frag_shader.get(), "main"}
			};
			vk::PipelineVertexInputStateCreateInfo vertex_input_state;
			vk::PipelineInputAssemblyStateCreateInfo input_assembly_state({}, vk::PrimitiveTopology::eTriangleList, false);
			vk::Viewport viewport(0.0f, 0.0f, swapchain_extent.width, swapchain_extent.height, 0.0f, 1.0f);
			vk::Rect2D scissor({0, 0}, swapchain_extent);
			vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
			vk::PipelineRasterizationStateCreateInfo rasterization_state(
				{},
				false,
				false,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack,
				vk::FrontFace::eClockwise,
				false,
				0.0f,
				0.0f,
				0.0f,
				1.0f
			);
			vk::PipelineMultisampleStateCreateInfo multisample_state;
			vk::PipelineColorBlendAttachmentState color_blend_attachment(
				false,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			);
			vk::PipelineColorBlendStateCreateInfo color_blend_state(
				{},
				false,
				vk::LogicOp::eCopy,
				1,
				&color_blend_attachment,
				{0.0f, 0.0f, 0.0f, 0.0f}
			);
			pipeline_layout = device->createPipelineLayoutUnique({});
			graphics_pipeline = device->createGraphicsPipelineUnique(nullptr, {
				{},
				2,
				shader_stages,
				&vertex_input_state,
				&input_assembly_state,
				nullptr,
				&viewport_state,
				&rasterization_state,
				&multisample_state,
				nullptr,
				&color_blend_state,
				nullptr,
				pipeline_layout.get(),
				render_pass.get(),
				0,
				nullptr,
				-1
			});
		}
		{ // Framebuffers
			swapchain_frame_buffers.reserve(swapchain_image_views.size());
			for (const auto& image_view : swapchain_image_views) {
				vk::ImageView attachments[] = { image_view.get() };
				swapchain_frame_buffers.push_back(device->createFramebufferUnique({
					{},
					render_pass.get(),
					1,
					attachments,
					swapchain_extent.width,
					swapchain_extent.height,
					1
				}));
			}
		}
		{ // Command Pool
			command_pool = device->createCommandPoolUnique({{}, graphics_queue_family});
		}
		{ // Command Buffers
			command_buffers = device->allocateCommandBuffersUnique({
				command_pool.get(),
				vk::CommandBufferLevel::ePrimary,
				(uint32_t) swapchain_frame_buffers.size()
			});
			for (size_t i = 0; i < command_buffers.size(); i++) {
				vk::ClearValue clear_value(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
				vk::CommandBufferBeginInfo command_buffer_begin_info;
				command_buffers[i]->begin(&command_buffer_begin_info);
				command_buffers[i]->beginRenderPass({
						render_pass.get(),
						swapchain_frame_buffers[i].get(),
						{{0, 0}, swapchain_extent},
						1,
						&clear_value
					},
					vk::SubpassContents::eInline
				);
				command_buffers[i]->bindPipeline(
					vk::PipelineBindPoint::eGraphics,
					graphics_pipeline.get()
				);
				command_buffers[i]->draw(3, 1, 0, 0);
				command_buffers[i]->endRenderPass();
				command_buffers[i]->end();
			}
		}
		{ // Sync Objects
			image_available_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
			render_finished_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
			in_flight_fences.reserve(MAX_FRAMES_IN_FLIGHT);
			images_in_flight.resize(swapchain_images.size(), nullptr);
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				image_available_semaphores.push_back(device->createSemaphoreUnique({}));
				render_finished_semaphores.push_back(device->createSemaphoreUnique({}));
				in_flight_fences.push_back(device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled}));
			}
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
