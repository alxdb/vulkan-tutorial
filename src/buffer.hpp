#pragma once

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

vk::raii::DeviceMemory allocateMemory(const vk::raii::Device &,
                                      const vk::raii::PhysicalDevice &,
                                      const vk::raii::Buffer &,
                                      vk::MemoryPropertyFlags);

template <typename T>
struct Buffer {
  const size_t size;
  const vk::raii::Buffer buffer;
  const vk::raii::DeviceMemory memory;

  Buffer(const vk::raii::Device &device,
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
};

template <typename T>
struct HostBuffer : Buffer<T> {
  const std::vector<T> data;

  HostBuffer(const vk::raii::Device &device,
             const vk::raii::PhysicalDevice &physicalDevice,
             const std::vector<T> &data,
             vk::BufferUsageFlags usage)
      : data(data),
        Buffer<T>(device,
                  physicalDevice,
                  sizeof(T) * data.size(),
                  usage,
                  vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible) {}

  void copyData() const {
    void *mappedMemory = this->memory.mapMemory(0, this->size);
    memcpy(mappedMemory, this->data.data(), this->size);
    this->memory.unmapMemory();
  }
};

struct Vertex {
  static vk::VertexInputBindingDescription bindingDescription;
  static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;

  glm::vec2 pos;
  glm::vec3 color;
};

struct VertexBuffer {
  const HostBuffer<Vertex> stagingBuffer;
  const Buffer<Vertex> deviceBuffer;

  VertexBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const std::vector<Vertex> &);

  void copyData(const vk::raii::Device &, const vk::raii::CommandPool &, const vk::raii::Queue &) const;
};