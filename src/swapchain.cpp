#include <iostream>

#include "swapchain.hpp"

vk::Extent2D determineExtent(const vkfw::Window &window, const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    auto [realWidth, realHeight] = static_cast<std::tuple<uint32_t, uint32_t>>(window.getFramebufferSize());
    return {
        std::clamp(realWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(realHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
  }
}

uint32_t determineMinImageCount(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.maxImageCount == 0) {
    return capabilities.minImageCount + 1;
  } else {
    return capabilities.maxImageCount;
  }
}

std::vector<vk::raii::ImageView> createImages(const vk::raii::SwapchainKHR &swapchain,
                                              const vk::raii::Device &device,
                                              const vk::Format &format) {
  auto swapchainImages = swapchain.getImages();

  std::vector<vk::raii::ImageView> result;
  result.reserve(swapchainImages.size());

  std::ranges::transform(swapchainImages, std::back_inserter(result), [&](const auto &image) {
    return device.createImageView(vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    });
  });

  return result;
}

vk::raii::SwapchainKHR createSwapchain(const vkfw::Window &window,
                                       const vk::raii::SurfaceKHR &surface,
                                       const Device &device,
                                       const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
                                       const vk::Extent2D &extent,
                                       const vk::SwapchainKHR &oldSwapchain) {
  return device.handle.createSwapchainKHR({
      .surface = *surface,
      .minImageCount = determineMinImageCount(surfaceCapabilities),
      .imageFormat = device.details.format.format,
      .imageColorSpace = device.details.format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = device.details.presentMode,
      .clipped = true,
      .oldSwapchain = oldSwapchain,
  });
}

std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::RenderPass &renderPass,
                                                      const vk::raii::Device &device,
                                                      const std::vector<vk::raii::ImageView> &images,
                                                      const vk::Extent2D &extent) {
  std::vector<vk::raii::Framebuffer> result;
  result.reserve(images.size());

  std::ranges::transform(images, std::back_inserter(result), [&](const auto &view) {
    std::array<vk::ImageView, 1> attachments = {*view};

    auto createInfo =
        vk::FramebufferCreateInfo{
            .renderPass = *renderPass,
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        }
            .setAttachments(attachments);

    return device.createFramebuffer(createInfo);
  });

  return result;
}

Swapchain::Swapchain(const vkfw::Window &window,
                     const vk::raii::SurfaceKHR &surface,
                     const Device &device,
                     const vk::raii::RenderPass &renderPass,
                     const vk::SwapchainKHR &swapchain)
    : surfaceCapabilities(device.details.physicalDevice.getSurfaceCapabilitiesKHR(*surface)),
      extent(determineExtent(window, surfaceCapabilities)),
      handle(device.handle.createSwapchainKHR({
          .surface = *surface,
          .minImageCount = determineMinImageCount(surfaceCapabilities),
          .imageFormat = device.details.format.format,
          .imageColorSpace = device.details.format.colorSpace,
          .imageExtent = extent,
          .imageArrayLayers = 1,
          .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
          .imageSharingMode = vk::SharingMode::eExclusive,
          .preTransform = surfaceCapabilities.currentTransform,
          .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
          .presentMode = device.details.presentMode,
          .clipped = true,
          .oldSwapchain = swapchain,
      })),
      images(createImages(handle, device.handle, device.details.format.format)),
      framebuffers(createFramebuffers(renderPass, device.handle, images, extent)) {}

Swapchain::Swapchain(const vkfw::Window &window,
                     const vk::raii::SurfaceKHR &surface,
                     const Device &device,
                     const vk::raii::RenderPass &renderPass)
    : Swapchain(window, surface, device, renderPass, VK_NULL_HANDLE) {}
