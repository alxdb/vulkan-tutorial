#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "pipeline.hpp"

struct Frame {
  const vk::raii::CommandBuffer commandBuffer;
  const vk::raii::DescriptorSet descriptorSet;

  const vk::raii::Semaphore imageAvailable;
  const vk::raii::Semaphore renderFinished;
  const vk::raii::Fence inFlight;

  const DynamicHostBuffer<UniformBufferObject> uniformBuffer;

  Frame(vk::raii::CommandBuffer &&,
        vk::raii::DescriptorSet &&,
        const vk::raii::Device &,
        const vk::raii::PhysicalDevice &);
};
