#include <bitset>

#include "vertex.hpp"

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

vk::raii::DeviceMemory allocateMemory(const vk::raii::Device &device,
                                      const vk::raii::PhysicalDevice &physicalDevice,
                                      const vk::raii::Buffer &buffer) {
  auto memoryRequirements = buffer.getMemoryRequirements();
  auto requiredPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  std::bitset<32> memoryTypeBits = memoryRequirements.memoryTypeBits;

  auto memoryProperties = physicalDevice.getMemoryProperties();
  uint32_t memoryTypeIndex = 0;
  for (const auto &memoryType : memoryProperties.memoryTypes) {
    if (memoryTypeBits[memoryTypeIndex] &&
        ((memoryType.propertyFlags & requiredPropertyFlags) == requiredPropertyFlags)) {
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

VertexBuffer::VertexBuffer(const vk::raii::Device &device,
                           const vk::raii::PhysicalDevice &physicalDevice,
                           const std::vector<Vertex> &vertices)
    : size(sizeof(Vertex) * vertices.size()),
      buffer(device.createBuffer({
          .size = size,
          .usage = vk::BufferUsageFlagBits::eVertexBuffer,
          .sharingMode = vk::SharingMode::eExclusive,
      })),
      memory(allocateMemory(device, physicalDevice, buffer)) {
  buffer.bindMemory(*memory, 0);

  void *mappedMemory = memory.mapMemory(0, size);
  memcpy(mappedMemory, vertices.data(), size);
  memory.unmapMemory();
}
