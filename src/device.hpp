#pragma once

#include <cstdint>

#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

#include <vulkan/vulkan_raii.hpp>

#include "surface_details.hpp"

struct Device {
  struct Details {
    const uint32_t queueFamilyIndex;
    const vk::raii::PhysicalDevice physicalDevice;
    const SurfaceDetails surfaceDetails;
  };

  const Details details;
  const vk::raii::Device handle;
  const vk::raii::Queue queue;

  Device(const vk::raii::Instance &, const vk::raii::SurfaceKHR &);
};
