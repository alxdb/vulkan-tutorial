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

vk::VertexInputBindingDescription Vertex::bindingDescription = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = vk::VertexInputRate::eVertex,
};

std::array<vk::VertexInputAttributeDescription, 2> Vertex::attributeDescriptions = {
    vk::VertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(Vertex, pos),
    },
    vk::VertexInputAttributeDescription{
        .location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(Vertex, color),
    },
};
