#include <iostream>

#include "swapchain.hpp"

vk::SurfaceFormatKHR pickSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats) {
  auto preferedFormat = vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
  auto surfaceFormat = std::ranges::find(formats, preferedFormat);
  if (surfaceFormat != formats.end()) {
    return *surfaceFormat;
  } else {
    return formats.back();
  }
}

vk::PresentModeKHR pickPresentMode(const std::vector<vk::PresentModeKHR> &presentModes) {
  auto preferredMode = vk::PresentModeKHR::eMailbox;
  auto presentMode = std::ranges::find(presentModes, preferredMode);
  if (presentMode != presentModes.end()) {
    return *presentMode;
  } else {
    return vk::PresentModeKHR::eFifo;
  }
}

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

Swapchain::Details determineDetails(const vkfw::Window &window, const SurfaceDetails &surfaceDetails) {
  auto surfaceFormat = pickSurfaceFormat(surfaceDetails.formats);
  return Swapchain::Details{
      .format = surfaceFormat.format,
      .colorSpace = surfaceFormat.colorSpace,
      .extent = determineExtent(window, surfaceDetails.capabilities),
      .presentMode = pickPresentMode(surfaceDetails.presentModes),
  };
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

Swapchain::Swapchain(const vkfw::Window &window,
                     const vk::raii::SurfaceKHR &surface,
                     const vk::raii::Device &device,
                     const SurfaceDetails &surfaceDetails)
    : details(determineDetails(window, surfaceDetails)),
      handle(device.createSwapchainKHR({
          .surface = *surface,
          .minImageCount = determineMinImageCount(surfaceDetails.capabilities),
          .imageFormat = details.format,
          .imageColorSpace = details.colorSpace,
          .imageExtent = details.extent,
          .imageArrayLayers = 1,
          .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
          .imageSharingMode = vk::SharingMode::eExclusive,
          .preTransform = surfaceDetails.capabilities.currentTransform,
          .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
          .presentMode = details.presentMode,
          .clipped = true,
          .oldSwapchain = VK_NULL_HANDLE,
      })),
      images(createImages(handle, device, details.format)) {}

std::vector<vk::raii::Framebuffer> Swapchain::createFramebuffers(const vk::raii::RenderPass &renderPass,
                                                                 const vk::raii::Device &device) const {
  std::vector<vk::raii::Framebuffer> result;
  result.reserve(images.size());

  std::ranges::transform(images, std::back_inserter(result), [&](const auto &view) {
    std::array<vk::ImageView, 1> attachments = {*view};

    auto createInfo =
        vk::FramebufferCreateInfo{
            .renderPass = *renderPass,
            .width = details.extent.width,
            .height = details.extent.height,
            .layers = 1,
        }
            .setAttachments(attachments);

    return device.createFramebuffer(createInfo);
  });

  return result;
}

void Swapchain::recreate(const vkfw::Window &window,
                         const vk::raii::SurfaceKHR &surface,
                         const vk::raii::Device &device,
                         const SurfaceDetails &surfaceDetails) {
  device.waitIdle();

  details = determineDetails(window, surfaceDetails);
  handle = device.createSwapchainKHR({
      .surface = *surface,
      .minImageCount = determineMinImageCount(surfaceDetails.capabilities),
      .imageFormat = details.format,
      .imageColorSpace = details.colorSpace,
      .imageExtent = details.extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceDetails.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = details.presentMode,
      .clipped = true,
      .oldSwapchain = *handle,
  });
  images = createImages(handle, device, details.format);
}
