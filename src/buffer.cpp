#include <bitset>

#include "buffer.hpp"

vk::raii::DeviceMemory allocateMemory(const vk::raii::Device &device,
                                      const vk::raii::PhysicalDevice &physicalDevice,
                                      const vk::raii::Buffer &buffer,
                                      vk::MemoryPropertyFlags memoryPropertyFlags) {
  auto memoryRequirements = buffer.getMemoryRequirements();
  std::bitset<32> memoryTypeBits = memoryRequirements.memoryTypeBits;

  auto memoryProperties = physicalDevice.getMemoryProperties();
  uint32_t memoryTypeIndex = 0;
  for (const auto &memoryType : memoryProperties.memoryTypes) {
    if (memoryTypeBits[memoryTypeIndex] && ((memoryType.propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)) {
      break;
    } else {
      memoryTypeIndex++;
    }
  }

  return device.allocateMemory({
      .allocationSize = memoryRequirements.size,
      .memoryTypeIndex = memoryTypeIndex,
  });
}