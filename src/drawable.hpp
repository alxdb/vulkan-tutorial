#pragma once

#include "buffer.hpp"
#include "pipeline.hpp"

struct Drawable {
  const std::vector<Vertex> vertices;
  const std::vector<Index> indices;
};

struct DrawableBuffers {
  const StagedBuffer<std::vector<Vertex>> vertexBuffer;
  const StagedBuffer<std::vector<Index>> indexBuffer;

  DrawableBuffers(const vk::raii::Device &, const vk::raii::PhysicalDevice &, const Drawable &);

  void copyData(const vk::raii::Device &, const vk::raii::CommandPool &, const vk::raii::Queue &) const;
};