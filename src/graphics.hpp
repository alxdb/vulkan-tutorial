#pragma once

#include <vector>

#ifndef VKFW_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS
#endif

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "base.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"

class Graphics {

private:
  const Base base;
  const Device device;
  const Swapchain swapchain;
  const Pipeline pipeline;
  std::vector<vk::raii::Framebuffer> framebuffers;
  vk::raii::CommandPool commandPool;
  vk::raii::CommandBuffer commandBuffer;
  vk::raii::Semaphore imageAvailable;
  vk::raii::Semaphore renderFinished;
  vk::raii::Fence inFlight;

  std::vector<vk::raii::Framebuffer> createFramebuffers() const;

  void recordCommandBuffer(size_t) const;

public:
  Graphics(const vkfw::Window &window)
      : base(window),
        device(base.instance, base.surface),
        swapchain(window, base.surface, device.handle, device.details.surfaceDetails),
        pipeline(swapchain.details.format, device.handle),
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
