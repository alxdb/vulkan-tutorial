#pragma once

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "device.hpp"

struct Swapchain {
  vk::SurfaceCapabilitiesKHR surfaceCapabilities;
  vk::Extent2D extent;
  vk::raii::SwapchainKHR handle;
  std::vector<vk::raii::ImageView> images;

  Swapchain(const vkfw::Window &, const vk::raii::SurfaceKHR &, const Device &device);
  Swapchain(const vkfw::Window &, const vk::raii::SurfaceKHR &, const Device &device, const vk::SwapchainKHR &);

  [[nodiscard]] std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::RenderPass &,
                                                                      const vk::raii::Device &) const;
};
