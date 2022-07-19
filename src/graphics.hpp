#pragma once

#include <vector>

#ifndef VKFW_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS
#endif

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "base.hpp"
#include "device.hpp"
#include "frame.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

class Graphics {

  const Base base;
  const Device device;
  const Swapchain swapchain;
  const Pipeline pipeline;

  const std::vector<vk::raii::Framebuffer> framebuffers;
  const std::array<Frame, 2> frames;
  uint32_t currentFrameIndex = 0;

  void recordCommandBuffer(const vk::raii::CommandBuffer &, size_t) const;

public:
  Graphics(const vkfw::Window &window)
      : base(window),
        device(base.instance, base.surface),
        swapchain(window, base.surface, device.handle, device.details.surfaceDetails),
        pipeline(swapchain.details.format, device.handle),
        framebuffers(swapchain.createFramebuffers(pipeline.renderPass, device.handle)),
        frames(device.createFrames()) {}

  void draw();
  void waitIdle() const { device.handle.waitIdle(); };
};
