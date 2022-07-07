#include "graphics.hpp"

#include <iostream>
#include <iterator>
#include <ranges>
#include <unordered_set>

#include "vertex_shader.h"

const std::array<const char *, 1> REQUIRED_DEVICE_EXTENSION_NAMES = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

vk::raii::Instance Graphics::createInstance() {
  const std::array<const char *, 1> requiredLayers = {
      "VK_LAYER_KHRONOS_validation",
  };
  auto requiredExtensions = vkfw::getRequiredInstanceExtensions();

  vk::ApplicationInfo applicationInfo{.apiVersion = VK_API_VERSION_1_1};

  return {
      context,
      vk::InstanceCreateInfo{.pApplicationInfo = &applicationInfo}
          .setPEnabledExtensionNames(requiredExtensions)
          .setPEnabledLayerNames(requiredLayers),
  };
}

vk::raii::PhysicalDevice Graphics::pickPhysicalDevice() {
  auto isSuitable = [&](const vk::raii::PhysicalDevice &physicalDevice) {
    auto deviceExtensionsProperties = physicalDevice.enumerateDeviceExtensionProperties();

    std::unordered_set<std::string> availableExtensionNames;
    std::ranges::transform(deviceExtensionsProperties,
                           std::inserter(availableExtensionNames, availableExtensionNames.begin()),
                           [](auto props) { return props.extensionName; });
    for (auto extensionName : REQUIRED_DEVICE_EXTENSION_NAMES) {
      if (!availableExtensionNames.contains(extensionName)) {
        return false;
      }
    }
    swapchainSupportDetails = {physicalDevice, surface};
    if (swapchainSupportDetails.formats.empty() || swapchainSupportDetails.presentModes.empty()) {
      return false;
    }

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    auto queueFamily = std::ranges::find_if(queueFamilies, [](const auto &queueFamily) {
      return (bool)(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
    });
    if (queueFamily == queueFamilies.end()) {
      return false;
    }

    queueFamilyIndex = std::distance(queueFamilies.begin(), queueFamily);
    return static_cast<bool>(physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surface));
  };

  auto physicalDevices = instance.enumeratePhysicalDevices();
  auto physicalDevice = std::ranges::find_if(physicalDevices, isSuitable);
  if (physicalDevice == physicalDevices.end()) {
    throw std::runtime_error("No suitabled devices");
  } else {
    return *physicalDevice;
  }
}

vk::raii::Device Graphics::createDevice() {
  std::array<float, 1> queuePriorities{1.0};
  auto queueCreateInfos = {
      vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = queueFamilyIndex,
          .queueCount = 1,
      }
          .setQueuePriorities(queuePriorities),
  };
  return {
      physicalDevice,
      vk::DeviceCreateInfo{}
          .setPEnabledExtensionNames(REQUIRED_DEVICE_EXTENSION_NAMES)
          .setQueueCreateInfos(queueCreateInfos),
  };
}

vk::raii::SwapchainKHR Graphics::createSwapchain(const vkfw::Window &window) {
  if (auto it = std::ranges::find(swapchainSupportDetails.formats,
                                  vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear});
      it != swapchainSupportDetails.formats.end()) {
    surfaceFormat = *it;
  } else {
    surfaceFormat = swapchainSupportDetails.formats.back();
  }

  vk::PresentModeKHR presentMode;
  if (auto it = std::ranges::find(swapchainSupportDetails.presentModes, vk::PresentModeKHR::eMailbox);
      it != swapchainSupportDetails.presentModes.end()) {
    presentMode = *it;
  } else {
    presentMode = vk::PresentModeKHR::eFifo;
  }

  if (swapchainSupportDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    swapExtent = swapchainSupportDetails.capabilities.currentExtent;
  } else {
    auto [realWidth, realHeight] = static_cast<std::tuple<uint32_t, uint32_t>>(window.getFramebufferSize());
    swapExtent = vk::Extent2D{
        std::clamp(realWidth,
                   swapchainSupportDetails.capabilities.minImageExtent.width,
                   swapchainSupportDetails.capabilities.maxImageExtent.width),
        std::clamp(realHeight,
                   swapchainSupportDetails.capabilities.minImageExtent.height,
                   swapchainSupportDetails.capabilities.maxImageExtent.height),
    };
  }

  uint32_t minImageCount;
  if (swapchainSupportDetails.capabilities.maxImageCount == 0) {
    minImageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
  } else {
    minImageCount = swapchainSupportDetails.capabilities.maxImageCount;
  }

  auto result = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR{
      .surface = *surface,
      .minImageCount = minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = swapExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = swapchainSupportDetails.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true,
      .oldSwapchain = swapchainCreated ? *swapchain : VK_NULL_HANDLE,
  });
  swapchainCreated = true;
  return result;
}

std::vector<vk::raii::ImageView> Graphics::createImageViews() {
  std::vector<vk::raii::ImageView> result;
  result.reserve(swapchainImages.size());

  std::ranges::transform(swapchainImages, std::back_inserter(result), [&](const auto &image) {
    return device.createImageView(vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = surfaceFormat.format,
          .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
        }
    });
  });

  return result;
}
