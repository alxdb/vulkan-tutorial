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

Buffer::Buffer(const vk::raii::Device &device,
               const vk::raii::PhysicalDevice &physicalDevice,
               size_t size,
               vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags memoryProperties)
    : size(size),
      buffer(device.createBuffer({
          .size = size,
          .usage = usage,
          .sharingMode = vk::SharingMode::eExclusive,
      })),
      memory(allocateMemory(device, physicalDevice, buffer, memoryProperties)) {
  buffer.bindMemory(*memory, 0);
}
