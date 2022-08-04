#include "graphics.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include <ranges>

Graphics::Graphics(const vkfw::Window &window)
    : base(window),
      device(base.instance, base.surface),
      pipeline(device.details.format.format, device.handle),
      frames(device.createFrames(pipeline.descriptorSetLayout)),
      vertexBuffer(device.handle, device.details.physicalDevice, vertices, vk::BufferUsageFlagBits::eVertexBuffer),
      indexBuffer(device.handle, device.details.physicalDevice, indices, vk::BufferUsageFlagBits::eIndexBuffer),
      swapchain(window, base.surface, device, pipeline.renderPass) {
  window.callbacks()->on_framebuffer_resize = [&](const vkfw::Window &, size_t, size_t) { recreateSwapchain(window); };
  vertexBuffer.copyData(device.handle, device.commandPool, device.queue);
  indexBuffer.copyData(device.handle, device.commandPool, device.queue);
}

void Graphics::recordCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, size_t framebufferIndex) const {
  std::array<vk::ClearValue, 1> clearValues = {{{{{{0.0f, 0.0f, 0.0f, 1.0f}}}}}};
  std::array<vk::Viewport, 1> viewports = {vk::Viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain.extent.width),
      .height = static_cast<float>(swapchain.extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  }};
  std::array<vk::Rect2D, 1> scissors = {vk::Rect2D{
      .offset = {0, 0},
      .extent = swapchain.extent,
  }};

  commandBuffer.begin({});
  commandBuffer.beginRenderPass(
      vk::RenderPassBeginInfo{
          .renderPass = *pipeline.renderPass,
          .framebuffer = *swapchain.framebuffers[framebufferIndex],
          .renderArea =
              {
                  .offset = {0, 0},
                  .extent = swapchain.extent,
              },
      }
          .setClearValues(clearValues),
      vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.handle);
  commandBuffer.bindVertexBuffers(0, {*vertexBuffer.deviceBuffer.buffer}, {0});
  commandBuffer.bindIndexBuffer(*indexBuffer.deviceBuffer.buffer, 0, vk::IndexType::eUint16);
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, *pipeline.pipelineLayout, 0, {*frames[currentFrameIndex].descriptorSet}, {});
  commandBuffer.setViewport(0, viewports);
  commandBuffer.setScissor(0, scissors);
  commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
  commandBuffer.endRenderPass();
  commandBuffer.end();
}

void Graphics::recreateSwapchain(const vkfw::Window &window) {
  waitIdle();
  swapchain = Swapchain(window, base.surface, device, pipeline.renderPass, *swapchain.handle);
}

void Graphics::updateUbo() {
  static auto start = std::chrono::high_resolution_clock::now();
  auto current = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration<float, std::chrono::seconds::period>(current - start).count();
  float aspectRatio = (float)swapchain.extent.width / (float)swapchain.extent.height;

  ubo.model = glm::rotate(glm::mat4(1.0f), delta * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
}

void Graphics::draw(const vkfw::Window &window) {

  const Frame &currentFrame = frames[currentFrameIndex];
  updateUbo();
  currentFrame.uniformBuffer.copyData(ubo);

  auto fenceResult = device.handle.waitForFences({*currentFrame.inFlight}, true, std::numeric_limits<uint64_t>::max());
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("Failed waiting for fence");
  }

  auto [acquireResult, imageIndex] =
      swapchain.handle.acquireNextImage(std::numeric_limits<uint64_t>::max(), *currentFrame.imageAvailable);

  if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
    std::cerr << "Image Out Of Date\n";
    recreateSwapchain(window);
    return;
  } else if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("Failed acquire");
  }

  // must occur after swapchain recreation, due to early return
  device.handle.resetFences({*currentFrame.inFlight});

  currentFrame.commandBuffer.reset();
  recordCommandBuffer(currentFrame.commandBuffer, imageIndex);

  auto waitStages = {static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
  device.queue.submit(vk::SubmitInfo{}
                          .setWaitSemaphores(*currentFrame.imageAvailable)
                          .setWaitDstStageMask(waitStages)
                          .setCommandBuffers(*currentFrame.commandBuffer)
                          .setSignalSemaphores(*currentFrame.renderFinished),
                      *currentFrame.inFlight);

  auto swapchains = {*swapchain.handle};
  auto imageIndices = {imageIndex};
  auto presentResult = device.queue.presentKHR(vk::PresentInfoKHR{}
                                                   .setWaitSemaphores(*currentFrame.renderFinished)
                                                   .setSwapchains(swapchains)
                                                   .setImageIndices(imageIndices));
  if (presentResult == vk::Result::eErrorOutOfDateKHR) {
    std::cerr << "Image Out Of Date\n";
    recreateSwapchain(window);
  } else if (presentResult != vk::Result::eSuccess && presentResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("Failed presentation");
  }

  currentFrameIndex = currentFrameIndex ^ 1;
}
