#pragma once

#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

#ifndef VKFW_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS
#endif

#include <vector>

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "device.hpp"

class Graphics {

private:
  const vk::raii::Context context;
  const vk::raii::Instance instance;
  const vk::raii::SurfaceKHR surface;
  const Device device;
  bool swapchainCreated = false;      // set by createSwapchain
  vk::SurfaceFormatKHR surfaceFormat; // set by createSwapchain
  vk::Extent2D swapExtent;            // set by createSwapchain
  vk::raii::SwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<vk::raii::ImageView> imageViews;
  vk::raii::PipelineLayout pipelineLayout;
  vk::raii::RenderPass renderPass;
  vk::raii::Pipeline pipeline;
  std::vector<vk::raii::Framebuffer> framebuffers;
  vk::raii::CommandPool commandPool;
  vk::raii::CommandBuffer commandBuffer;
  vk::raii::Semaphore imageAvailable;
  vk::raii::Semaphore renderFinished;
  vk::raii::Fence inFlight;

  vk::raii::Instance createInstance() const;
  vk::raii::PhysicalDevice pickPhysicalDevice();
  vk::raii::Device createDevice() const;
  vk::raii::SwapchainKHR createSwapchain(const vkfw::Window &);
  std::vector<vk::raii::ImageView> createImageViews() const;
  vk::raii::RenderPass createRenderPass() const;
  vk::raii::Pipeline createPipeline() const;
  std::vector<vk::raii::Framebuffer> createFramebuffers() const;

  void recordCommandBuffer(size_t) const;

public:
  Graphics(const vkfw::Window &window)
      : instance(createInstance()),
        surface(instance, vkfw::createWindowSurface(*instance, window)),
        device(instance, surface),
        swapchain(createSwapchain(window)),
        swapchainImages(swapchain.getImages()),
        imageViews(createImageViews()),
        pipelineLayout(device.handle.createPipelineLayout({})),
        renderPass(createRenderPass()),
        pipeline(createPipeline()),
        framebuffers(createFramebuffers()),
        commandPool(device.handle.createCommandPool({
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = device.details.queueFamilyIndex,
        })),
        commandBuffer(std::move(device.handle.allocateCommandBuffers({
            .commandPool = *commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        })[0])),
        imageAvailable(device.handle.createSemaphore({})),
        renderFinished(device.handle.createSemaphore({})),
        inFlight(device.handle.createFence({.flags = vk::FenceCreateFlagBits::eSignaled})) {}

  void draw();
  void waitIdle() { device.handle.waitIdle(); };
};
