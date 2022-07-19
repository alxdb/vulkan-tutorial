#pragma once

#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

#include <vulkan/vulkan_raii.hpp>

struct Frame {
  const vk::raii::CommandBuffer commandBuffer;
  const vk::raii::Semaphore imageAvailable;
  const vk::raii::Semaphore renderFinished;
  const vk::raii::Fence inFlight;

  Frame(vk::raii::CommandBuffer &&, const vk::raii::Device &);
};
