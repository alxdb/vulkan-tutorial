#pragma once

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "surface_details.hpp"

struct Swapchain {
  struct Details {
    const vk::Format format;
    const vk::ColorSpaceKHR colorSpace;
    const vk::Extent2D extent;
    const vk::PresentModeKHR presentMode;
  };

  const Details details;
  const vk::raii::SwapchainKHR handle;
  const std::vector<vk::raii::ImageView> images;

  Swapchain(const vkfw::Window &, const vk::raii::SurfaceKHR &, const vk::raii::Device &, const SurfaceDetails &);

  std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::RenderPass &, const vk::raii::Device &) const;
};
