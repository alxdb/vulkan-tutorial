#pragma once

#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

struct SwapchainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;

  SwapchainSupportDetails() : capabilities(), formats(), presentModes() {}
  SwapchainSupportDetails(const vk::raii::PhysicalDevice &physicalDevice,
                          const vk::raii::SurfaceKHR &surface)
      : capabilities(physicalDevice.getSurfaceCapabilitiesKHR(*surface)),
        formats(physicalDevice.getSurfaceFormatsKHR(*surface)),
        presentModes(physicalDevice.getSurfacePresentModesKHR(*surface)) {}
};

class Graphics {
private:
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::SurfaceKHR surface;
  uint32_t queueFamilyIndex = -1;                  // set by pickPhysicalDevice
  SwapchainSupportDetails swapchainSupportDetails; // set by pickPhysicalDevice
  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;
  vk::raii::Queue queue;
  bool swapchainCreated = false;
  vk::raii::SwapchainKHR swapchain;

  vk::raii::Instance createInstance();
  vk::raii::PhysicalDevice pickPhysicalDevice();
  vk::raii::Device createDevice();
  vk::raii::SwapchainKHR createSwapchain(const vkfw::Window &);

public:
  Graphics(const vkfw::Window &window)
      : instance(createInstance()),
        surface(instance, vkfw::createWindowSurface(*instance, window)),
        physicalDevice(pickPhysicalDevice()),
        device(createDevice()),
        queue(device.getQueue(queueFamilyIndex, 0)),
        swapchain(createSwapchain(window)) {}
};
