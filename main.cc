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


class App {
	Window window;

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
		}
	}
	explicit App(const char * application_name) : window(application_name, 0, 0) {
		vk::UniqueInstance instance;
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
		vk::UniqueSurfaceKHR surface;
		{ // Window Surface
			surface = window.createSurface(instance);
		}
		uint32_t graphics_queue_family;
		vk::PhysicalDevice physical_device;
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
		vk::UniqueDevice device;
		vk::Queue graphics_queue;
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
		vk::UniqueSwapchainKHR swapchain;
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
		std::vector<vk::UniqueImageView> swapchain_image_views;
		{ // Image Views
			swapchain_image_views.resize(swapchain_images.size());
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
		}
		vk::UniquePipelineLayout pipeline_layout;
		{ // Graphics Pipeline
			vk::UniqueShaderModule vert = create_shader(device, "build/main.vert.spv");
			vk::UniqueShaderModule frag = create_shader(device, "build/main.frag.spv");
			vk::PipelineShaderStageCreateInfo shader_stages[] = {
				{{}, vk::ShaderStageFlagBits::eVertex, vert.get(), "main"},
				{{}, vk::ShaderStageFlagBits::eFragment, frag.get(), "main"}
			};
			vk::PipelineVertexInputStateCreateInfo vertex_input_info;
			vk::PipelineInputAssemblyStateCreateInfo input_assembly_info({}, vk::PrimitiveTopology::eTriangleList, false);
			vk::Viewport viewport(0.0f, 0.0f, swapchain_extent.width, swapchain_extent.height, 0.0f, 1.0f);
			vk::Rect2D scissor({0, 0}, swapchain_extent);
			vk::PipelineViewportStateCreateInfo viewport_state_info({}, 1, &viewport, 1, &scissor);
			vk::PipelineRasterizationStateCreateInfo rasterizer(
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
			vk::PipelineMultisampleStateCreateInfo multisample_state_info{};
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
			vk::PipelineColorBlendStateCreateInfo color_blending_info(
				{},
				false,
				vk::LogicOp::eCopy,
				1,
				&color_blend_attachment,
				{0.0f, 0.0f, 0.0f, 0.0f}
			);
			pipeline_layout = device->createPipelineLayoutUnique({});
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
