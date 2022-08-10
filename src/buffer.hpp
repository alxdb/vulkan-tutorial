#pragma once

#include <ranges>

#include <vulkan/vulkan_raii.hpp>

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

template <std::ranges::contiguous_range R>
struct HostBuffer : Buffer {
  const R data;

  HostBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const R &, vk::BufferUsageFlags);

  void copyData() const;
};

template <typename T>
struct DynamicHostBuffer : Buffer {
  DynamicHostBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, vk::BufferUsageFlags);

  void copyData(const T &data) const;
};

template <std::ranges::contiguous_range R>
struct StagedBuffer {
  const HostBuffer<R> stagingBuffer;
  const Buffer deviceBuffer;

  StagedBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const R &, vk::BufferUsageFlags);

  void copyData(const vk::raii::Device &, const vk::raii::CommandPool &, const vk::raii::Queue &) const;
};

#include "buffer_impl.hpp"