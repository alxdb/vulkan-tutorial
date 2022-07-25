#include <iostream>

#include "base.hpp"

const auto REQUIRED_LAYER_NAMES = {
#ifndef NDEBUG
    "VK_LAYER_KHRONOS_validation",
#endif
};

const vk::ApplicationInfo APPLICATION_INFO = {
    .apiVersion = VK_API_VERSION_1_1,
};

vk::raii::Instance createInstance(const vk::raii::Context &context) {
  // implicit dependency on vkfw::Instance
  std::vector<const char *> requiredExtensions;
  for (const auto &requiredExtension : vkfw::getRequiredInstanceExtensions()) {
    requiredExtensions.push_back(requiredExtension);
  }
  requiredExtensions.push_back("VK_KHR_portability_enumeration");

  return {context,
          vk::InstanceCreateInfo{
              .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
              .pApplicationInfo = &APPLICATION_INFO,
          }
              .setPEnabledExtensionNames(requiredExtensions)
              .setPEnabledLayerNames(REQUIRED_LAYER_NAMES)};
}

Base::Base(const vkfw::Window &window)
    : instance(createInstance(context)), surface(instance, vkfw::createWindowSurface(*instance, window)) {}
