#pragma once

#include <vulkan/vulkan_raii.hpp>

struct Pipeline {
  const vk::raii::PipelineLayout layout;
  const vk::raii::RenderPass renderPass;
  const vk::raii::Pipeline handle;

  Pipeline(const vk::Format &, const vk::raii::Device &);
};
