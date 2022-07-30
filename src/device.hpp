#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "frame.hpp"

struct Device {
  struct Details {
    const uint32_t queueFamilyIndex;
    const vk::raii::PhysicalDevice physicalDevice;
    const vk::SurfaceFormatKHR format;
    const vk::PresentModeKHR presentMode;
  };

  const Details details;
  const vk::raii::Device handle;
  const vk::raii::Queue queue;
  const vk::raii::CommandPool commandPool;

  Device(const vk::raii::Instance &, const vk::raii::SurfaceKHR &);

  [[nodiscard]] std::array<Frame, 2> createFrames() const;
};
