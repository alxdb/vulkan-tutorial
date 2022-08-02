#pragma once

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

template <typename T>
struct Buffer {
  const size_t size;
  const vk::raii::Buffer buffer;
  const vk::raii::DeviceMemory memory;

  Buffer(const vk::raii::Device &,
         const vk::raii::PhysicalDevice &,
         size_t,
         vk::BufferUsageFlags,
         vk::MemoryPropertyFlags);
};

template <typename T>
struct HostBuffer : Buffer<T> {
  const std::vector<T> data;

  HostBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const std::vector<T> &, vk::BufferUsageFlags);

  void copyData() const;
};

struct Vertex {
  static vk::VertexInputBindingDescription bindingDescription;
  static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;

  glm::vec2 pos;
  glm::vec3 color;
};

template <typename T>
struct StagedBuffer {
  const HostBuffer<T> stagingBuffer;
  const Buffer<T> deviceBuffer;

  StagedBuffer(const vk::raii::Device &,
               const vk::raii::PhysicalDevice &,
               const std::vector<T> &,
               vk::BufferUsageFlags);

  void copyData(const vk::raii::Device &, const vk::raii::CommandPool &, const vk::raii::Queue &) const;
};

#include "buffer_impl.hpp"