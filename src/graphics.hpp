#pragma once

#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

#ifndef VKFW_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS
#endif

#include <vector>

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

struct SwapchainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;

  SwapchainSupportDetails() : capabilities(), formats(), presentModes() {}
  SwapchainSupportDetails(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface)
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
  vk::SurfaceFormatKHR surfaceFormat; // set by createSwapchain
  vk::Extent2D swapExtent;            // set by createSwapchain
  vk::raii::SwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<vk::raii::ImageView> imageViews;

  vk::raii::Instance createInstance();
  vk::raii::PhysicalDevice pickPhysicalDevice();
  vk::raii::Device createDevice();
  vk::raii::SwapchainKHR createSwapchain(const vkfw::Window &);
  std::vector<vk::raii::ImageView> createImageViews();

public:
  Graphics(const vkfw::Window &window)
      : instance(createInstance()),
        surface(instance, vkfw::createWindowSurface(*instance, window)),
        physicalDevice(pickPhysicalDevice()),
        device(createDevice()),
        queue(device.getQueue(queueFamilyIndex, 0)),
        swapchain(createSwapchain(window)),
        swapchainImages(swapchain.getImages()),
        imageViews(createImageViews()) {}
};
