#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "frame.hpp"
#include "surface_details.hpp"

struct Device {
  struct Details {
    const uint32_t queueFamilyIndex;
    const vk::raii::PhysicalDevice physicalDevice;
    SurfaceDetails surfaceDetails;

    void determineSurfaceCapabilities(const vk::raii::SurfaceKHR &surface) {
      surfaceDetails.determineCapabilities(physicalDevice, surface);
    }
  };

  Details details;
  const vk::raii::Device handle;
  const vk::raii::Queue queue;
  const vk::raii::CommandPool commandPool;

  Device(const vk::raii::Instance &, const vk::raii::SurfaceKHR &);

  [[nodiscard]] std::array<Frame, 2> createFrames() const;
};
