#pragma once

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
  static vk::VertexInputBindingDescription bindingDescription;
  static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;

  glm::vec2 pos;
  glm::vec3 color;
};

struct Pipeline {
  const vk::raii::PipelineLayout layout;
  const vk::raii::RenderPass renderPass;
  const vk::raii::Pipeline handle;

  Pipeline(const vk::Format &, const vk::raii::Device &);
};
