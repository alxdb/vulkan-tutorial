#pragma once

#include <vector>

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "base.hpp"
#include "device.hpp"
#include "frame.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

class Graphics {

  const Base base;
  Device device;
  Swapchain swapchain;
  const Pipeline pipeline;

  std::vector<vk::raii::Framebuffer> framebuffers;
  size_t currentFrameIndex = 0;
  const std::array<Frame, 2> frames;

  void recordCommandBuffer(const vk::raii::CommandBuffer &, size_t) const;
  void waitIdle() const { device.handle.waitIdle(); };

public:
  Graphics(const vkfw::Window &window)
      : base(window),
        device(base.instance, base.surface),
        swapchain(window, base.surface, device.handle, device.details.surfaceDetails),
        pipeline(swapchain.details.format, device.handle),
        framebuffers(swapchain.createFramebuffers(pipeline.renderPass, device.handle)),
        frames(device.createFrames()) {
    window.callbacks()->on_framebuffer_resize = [&](const vkfw::Window &, size_t, size_t) {
      recreateSwapchain(window);
    };
  }

  ~Graphics() { waitIdle(); };

  void draw(const vkfw::Window &);
  void recreateSwapchain(const vkfw::Window &);
};
