#pragma once

#include <vulkan/vulkan_raii.hpp>

struct SurfaceDetails {
  const vk::SurfaceCapabilitiesKHR capabilities;
  const std::vector<vk::SurfaceFormatKHR> formats;
  const std::vector<vk::PresentModeKHR> presentModes;
};
