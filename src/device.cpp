#include "device.hpp"

#include <optional>
#include <ranges>
#include <unordered_set>

const std::array<const char *, 1> REQUIRED_DEVICE_EXTENSION_NAMES = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const std::array<float, 1> QUEUE_PRIORITIES = {1.0};

std::optional<Device::Details> isSuitable(const vk::raii::PhysicalDevice &physicalDevice,
                                          const vk::raii::SurfaceKHR &surface) {
  auto deviceExtensionsProperties = physicalDevice.enumerateDeviceExtensionProperties();

  // supports required extensions
  std::unordered_set<std::string> availableExtensionNames;
  std::ranges::transform(deviceExtensionsProperties,
                         std::inserter(availableExtensionNames, availableExtensionNames.begin()),
                         [](const auto &props) { return props.extensionName; });
  for (auto extensionName : REQUIRED_DEVICE_EXTENSION_NAMES) {
    if (!availableExtensionNames.contains(extensionName)) {
      return std::nullopt;
    }
  }

  // has a suitable queue family
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();
  auto queueFamily = std::ranges::find_if(queueFamilies, [](const auto &queueFamily) {
    return static_cast<bool>(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
  });
  if (queueFamily == queueFamilies.end()) {
    return std::nullopt;
  }
  uint32_t queueFamilyIndex = std::distance(queueFamilies.begin(), queueFamily);

  // supports surface
  auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
  auto presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
  if (surfaceFormats.empty() || presentModes.empty() ||
      !physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surface)) {
    return std::nullopt;
  }

  return Device::Details{
      .queueFamilyIndex = queueFamilyIndex,
      .physicalDevice = physicalDevice,
      .surfaceDetails =
          {
              .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface),
              .formats = surfaceFormats,
              .presentModes = presentModes,
          },
  };
}

Device::Details findSuitableDevice(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface) {
  for (const auto &physicalDevice : instance.enumeratePhysicalDevices()) {
    if (auto details = isSuitable(physicalDevice, surface)) {
      return *details;
    }
  }
  throw std::runtime_error("No suitable devices");
}

vk::raii::Device createDevice(const Device::Details &details) {
  auto queueCreateInfos = {
      vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = details.queueFamilyIndex,
          .queueCount = 1,
      }
          .setQueuePriorities(QUEUE_PRIORITIES),
  };
  return {
      details.physicalDevice,
      vk::DeviceCreateInfo{}
          .setPEnabledExtensionNames(REQUIRED_DEVICE_EXTENSION_NAMES)
          .setQueueCreateInfos(queueCreateInfos),
  };
}

Device::Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface)
    : details(findSuitableDevice(instance, surface)),
      handle(createDevice(details)),
      queue(handle.getQueue(details.queueFamilyIndex, 0)),
      commandPool(handle.createCommandPool({
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex = details.queueFamilyIndex,
      })) {}

std::array<Frame, 2> Device::createFrames() const {
  auto commandBuffers = handle.allocateCommandBuffers({
      .commandPool = *commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 2,
  });

  return {Frame(std::move(commandBuffers[0]), handle), Frame(std::move(commandBuffers[1]), handle)};
}
