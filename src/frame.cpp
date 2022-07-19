#include "frame.hpp"

Frame::Frame(vk::raii::CommandBuffer &&commandBuffer, const vk::raii::Device &device)
    : commandBuffer(std::move(commandBuffer)),
      imageAvailable(device.createSemaphore({})),
      renderFinished(device.createSemaphore({})),
      inFlight(device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled})) {}
