#pragma once

#include <vulkan/vulkan_raii.hpp>

struct SurfaceDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  const std::vector<vk::SurfaceFormatKHR> formats;
  const std::vector<vk::PresentModeKHR> presentModes;

  void determineCapabilities(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface) {
    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  }
};
