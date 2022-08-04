#include "frame.hpp"

Frame::Frame(vk::raii::CommandBuffer &&commandBuffer,
             vk::raii::DescriptorSet &&descriptorSet,
             const vk::raii::Device &device,
             const vk::raii::PhysicalDevice &physicalDevice)
    : commandBuffer(std::move(commandBuffer)),
      descriptorSet(std::move(descriptorSet)),
      imageAvailable(device.createSemaphore({})),
      renderFinished(device.createSemaphore({})),
      inFlight(device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled})),
      uniformBuffer(device, physicalDevice, vk::BufferUsageFlagBits::eUniformBuffer) {
  auto descriptorBufferInfo = {vk::DescriptorBufferInfo{
      .buffer = *uniformBuffer.buffer,
      .offset = 0,
      .range = sizeof(UniformBufferObject),
  }};
  auto writeDescriptorSet =
      vk::WriteDescriptorSet{
          .dstSet = *this->descriptorSet,
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
      }
          .setBufferInfo(descriptorBufferInfo);
  device.updateDescriptorSets({writeDescriptorSet}, {});
}
