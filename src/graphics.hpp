#pragma once

#include <vector>

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "base.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "frame.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

class Graphics {
  const Base base;
  const Device device;
  const Pipeline pipeline;

  size_t currentFrameIndex = 0;
  const std::array<Frame, 2> frames;

  const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{+0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{+0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, +0.5f}, {1.0f, 1.0f, 1.0f}},
  };
  const StagedBuffer<Vertex> vertexBuffer;
  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
  const StagedBuffer<uint16_t> indexBuffer;
  UniformBufferObject ubo{};

  Swapchain swapchain;

  void recordCommandBuffer(const vk::raii::CommandBuffer &, size_t) const;
  void recreateSwapchain(const vkfw::Window &);
  void updateUbo();
  void waitIdle() const { device.handle.waitIdle(); };

public:
  Graphics(const vkfw::Window &window);
  ~Graphics() { waitIdle(); };

  void draw(const vkfw::Window &);
};
