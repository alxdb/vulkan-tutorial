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

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Pipeline {
  const vk::raii::DescriptorSetLayout descriptorSetLayout;
  const vk::raii::PipelineLayout pipelineLayout;
  const vk::raii::RenderPass renderPass;
  const vk::raii::Pipeline handle;

  Pipeline(const vk::Format &, const vk::raii::Device &);
};
