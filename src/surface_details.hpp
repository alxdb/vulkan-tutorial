#pragma once

#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

#include <vulkan/vulkan_raii.hpp>

struct SurfaceDetails {
  const vk::SurfaceCapabilitiesKHR capabilities;
  const std::vector<vk::SurfaceFormatKHR> formats;
  const std::vector<vk::PresentModeKHR> presentModes;
};
