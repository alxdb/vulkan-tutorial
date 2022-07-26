#pragma once

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "surface_details.hpp"

struct Swapchain {
  struct Details {
    vk::Format format;
    vk::ColorSpaceKHR colorSpace;
    vk::Extent2D extent;
    vk::PresentModeKHR presentMode;
  };

  Details details;
  vk::raii::SwapchainKHR handle;
  std::vector<vk::raii::ImageView> images;

  Swapchain(const vkfw::Window &, const vk::raii::SurfaceKHR &, const vk::raii::Device &, const SurfaceDetails &);

  std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::RenderPass &, const vk::raii::Device &) const;
  void recreate(const vkfw::Window &, const vk::raii::SurfaceKHR &, const vk::raii::Device &, const SurfaceDetails &);
};
