#pragma once

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
  static vk::VertexInputBindingDescription bindingDescription;
  static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;

  const glm::vec2 pos;
  const glm::vec3 color;
};

struct VertexBuffer {
  const size_t size;
  const vk::raii::Buffer buffer;
  const vk::raii::DeviceMemory memory;

  VertexBuffer(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const std::vector<Vertex> &);
};