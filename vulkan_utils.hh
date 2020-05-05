#pragma once
#include <filesystem>
#include <fstream>
#include <vulkan/vulkan.hpp>
#include "window.hh"

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
		layer_names.size(),
		layer_names.data(),
		extension_names.size(),
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
	const char* extension_names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::DeviceQueueCreateInfo queue_create_info({}, graphics_queue_family, 1, &queue_priority);
	vk::DeviceCreateInfo device_create_info({}, 1, &queue_create_info);
	device_create_info.setEnabledExtensionCount(1);
	device_create_info.setPpEnabledExtensionNames(extension_names);
	return physical_device.createDeviceUnique(device_create_info);
}

vk::UniqueSwapchainKHR create_swapchain(
	const vk::UniqueDevice& device,
	const vk::UniqueSurfaceKHR& surface,
	const vk::SurfaceCapabilitiesKHR& surface_capabilites,
	const vk::SurfaceFormatKHR& surface_format
) {
	vk::SwapchainCreateInfoKHR swapchain_create_info(
		{},
		surface.get(),
		surface_capabilites.minImageCount,
		surface_format.format,
		surface_format.colorSpace,
		surface_capabilites.currentExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0, nullptr,
		surface_capabilites.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::PresentModeKHR::eFifo,
		true
	);
	return device->createSwapchainKHRUnique(swapchain_create_info);
};

std::vector<vk::UniqueImageView> create_swapchain_image_views(
	const vk::UniqueDevice& device,
	const vk::UniqueSwapchainKHR& swapchain,
	const vk::SurfaceFormatKHR& surface_format
) {
	std::vector<vk::Image> swapchain_images = device->getSwapchainImagesKHR(swapchain.get());
	std::vector<vk::UniqueImageView> swapchain_image_views;
	swapchain_image_views.reserve(swapchain_images.size());
	for (const vk::Image& swapchain_image : swapchain_images) {
		vk::ImageViewCreateInfo image_view_create_info(
			{},
			swapchain_image,
			vk::ImageViewType::e2D,
			surface_format.format,
			{},
			{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		);
		swapchain_image_views.push_back(device->createImageViewUnique(image_view_create_info));
	}
	return swapchain_image_views;
}

vk::UniqueRenderPass create_render_pass(
	const vk::UniqueDevice& device,
	const vk::SurfaceFormatKHR& surface_format
) {
	vk::AttachmentDescription color_attachment_description(
		{},
		surface_format.format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);
	vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDependency subpass_dependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{},
		vk::AccessFlagBits::eColorAttachmentWrite
	);
	vk::SubpassDescription subpass_description(
		{},
		vk::PipelineBindPoint::eGraphics,
		0, nullptr,
		1, &color_attachment_ref
	);
	vk::RenderPassCreateInfo render_pass_create_info(
		{},
		1, &color_attachment_description,
		1, &subpass_description,
		1, &subpass_dependency
	);
	return device->createRenderPassUnique(render_pass_create_info);
}

vk::UniqueShaderModule create_shader(
	const vk::UniqueDevice& device,
	const std::filesystem::path& spirv_file
) {
	std::ifstream file(spirv_file, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}
	std::vector<char> code(file.tellg());
	file.seekg(0);
	file.read(code.data(), code.size());
	file.close();
	vk::ShaderModuleCreateInfo shader_module_create_info(
		{},
		code.size(),
		reinterpret_cast<const uint32_t*>(code.data())
	);
	return device->createShaderModuleUnique(shader_module_create_info);
}

vk::UniquePipeline create_pipeline(
	const vk::UniqueDevice& device,
	const vk::SurfaceCapabilitiesKHR& surface_capabilites,
	const vk::UniqueShaderModule& vert_shader,
	const vk::UniqueShaderModule& frag_shader,
	const vk::UniquePipelineLayout& pipeline_layout,
	const vk::UniqueRenderPass& render_pass
) {
	// Shaders
	vk::PipelineShaderStageCreateInfo shader_stage_create_infos[] = {
		{{}, vk::ShaderStageFlagBits::eVertex, vert_shader.get(), "main"},
		{{}, vk::ShaderStageFlagBits::eFragment, frag_shader.get(), "main"}
	};
	vk::PipelineVertexInputStateCreateInfo vertex_input_state;
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_state(
		{},
		vk::PrimitiveTopology::eTriangleList,
		false
	);
	// Viewport
	vk::Viewport viewport(
		0.0f, 0.0f,
		surface_capabilites.currentExtent.width, surface_capabilites.currentExtent.height,
		0.0f, 1.0f
	);
	vk::Rect2D scissor({0, 0}, surface_capabilites.currentExtent);
	vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
	// Rasterizer
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
	// Multisample
	vk::PipelineMultisampleStateCreateInfo multisample_state;
	// Color Blend
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
	// Pipeline
	vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info(
		{},
		2, shader_stage_create_infos,
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
		nullptr, -1
	);
	return device->createGraphicsPipelineUnique(nullptr, graphics_pipeline_create_info);
}

std::vector<vk::UniqueFramebuffer> create_frame_buffers(
	const vk::UniqueDevice& device,
	const vk::UniqueRenderPass& render_pass,
	const vk::SurfaceCapabilitiesKHR& surface_capabilites,
	const std::vector<vk::UniqueImageView>& swapchain_image_views
) {
	std::vector<vk::UniqueFramebuffer> swapchain_frame_buffers;
	swapchain_frame_buffers.reserve(swapchain_image_views.size());
	for (const auto& image_view : swapchain_image_views) {
		vk::ImageView attachments[] = { image_view.get() };
		vk::FramebufferCreateInfo frame_buffer_create_info(
			{},
			render_pass.get(),
			1,
			attachments,
			surface_capabilites.currentExtent.width,
			surface_capabilites.currentExtent.height,
			1
		);
		swapchain_frame_buffers.push_back(device->createFramebufferUnique(frame_buffer_create_info));
	}
	return swapchain_frame_buffers;
}

std::vector<vk::UniqueCommandBuffer> create_command_buffers(
	const vk::UniqueDevice& device,
	const vk::UniqueCommandPool& command_pool,
	const vk::UniqueRenderPass& render_pass,
	const vk::UniquePipeline& graphics_pipeline,
	const vk::SurfaceCapabilitiesKHR& surface_capabilites,
	const std::vector<vk::UniqueFramebuffer>& swapchain_frame_buffers
) {
	vk::CommandBufferAllocateInfo command_buffer_allocate_info(
		command_pool.get(),
		vk::CommandBufferLevel::ePrimary,
		swapchain_frame_buffers.size()
	);
	std::vector<vk::UniqueCommandBuffer> command_buffers =
		device->allocateCommandBuffersUnique(command_buffer_allocate_info);
	for (size_t i = 0; i < command_buffers.size(); i++) {
		vk::ClearValue clear_value(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
		vk::CommandBufferBeginInfo command_buffer_begin_info;
		command_buffers[i]->begin(&command_buffer_begin_info);
		vk::RenderPassBeginInfo render_pass_begin_info(
			render_pass.get(),
			swapchain_frame_buffers[i].get(),
			{{0, 0}, surface_capabilites.currentExtent},
			1, &clear_value
		);
		command_buffers[i]->beginRenderPass(
			render_pass_begin_info,
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
	return command_buffers;
}
