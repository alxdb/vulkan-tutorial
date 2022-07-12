#include "base.hpp"

const auto REQUIRED_LAYER_NAMES = {
#ifndef NDEBUG
    "VK_LAYER_KHRONOS_validation",
#endif
};

const vk::ApplicationInfo APPLICATION_INFO = {
    .apiVersion = VK_API_VERSION_1_1,
};

vk::InstanceCreateInfo instanceCreateInfo() {
  // implicit dependency on vkfw::Instance
  auto requiredExtensions = vkfw::getRequiredInstanceExtensions();
  return vk::InstanceCreateInfo{.pApplicationInfo = &APPLICATION_INFO}
      .setPEnabledExtensionNames(requiredExtensions)
      .setPEnabledLayerNames(REQUIRED_LAYER_NAMES);
}

Base::Base(const vkfw::Window &window)
    : instance(context, instanceCreateInfo()), surface(instance, vkfw::createWindowSurface(*instance, window)) {}
