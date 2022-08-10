#include "drawable.hpp"

DrawableBuffers::DrawableBuffers(const vk::raii::Device &device,
                                 const vk::raii::PhysicalDevice &physicalDevice,
                                 const Drawable &drawable)
    : vertexBuffer(device, physicalDevice, drawable.vertices, vk::BufferUsageFlagBits::eVertexBuffer),
      indexBuffer(device, physicalDevice, drawable.indices, vk::BufferUsageFlagBits::eIndexBuffer) {}

void DrawableBuffers::copyData(const vk::raii::Device &device,
                               const vk::raii::CommandPool &commandPool,
                               const vk::raii::Queue &queue) const {
  vertexBuffer.copyData(device, commandPool, queue);
  indexBuffer.copyData(device, commandPool, queue);
}
