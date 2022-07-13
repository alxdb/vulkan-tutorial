#include "graphics.hpp"

#include <ranges>

std::vector<vk::raii::Framebuffer> Graphics::createFramebuffers() const {
  std::vector<vk::raii::Framebuffer> result;
  result.reserve(swapchain.images.size());

  std::ranges::transform(swapchain.images, std::back_inserter(result), [&](const auto &view) {
    std::array<vk::ImageView, 1> attachments = {*view};
    auto createInfo =
        vk::FramebufferCreateInfo{
            .renderPass = *pipeline.renderPass,
            .width = swapchain.details.extent.width,
            .height = swapchain.details.extent.height,
            .layers = 1,
        }
            .setAttachments(attachments);
    return device.handle.createFramebuffer(createInfo);
  });

  return result;
}

void Graphics::recordCommandBuffer(size_t framebuffer_index) const {
  std::array<vk::ClearValue, 1> clearValues = {{{{{{0.0f, 0.0f, 0.0f, 1.0f}}}}}};
  std::array<vk::Viewport, 1> viewports = {vk::Viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain.details.extent.width),
      .height = static_cast<float>(swapchain.details.extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  }};
  std::array<vk::Rect2D, 1> scissors = {vk::Rect2D{
      .offset = {0, 0},
      .extent = swapchain.details.extent,
  }};

  commandBuffer.begin({});
  commandBuffer.beginRenderPass(
      vk::RenderPassBeginInfo{
          .renderPass = *pipeline.renderPass,
          .framebuffer = *framebuffers[framebuffer_index],
          .renderArea =
              {
                  .offset = {0, 0},
                  .extent = swapchain.details.extent,
              },
      }
          .setClearValues(clearValues),
      vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.handle);
  commandBuffer.setViewport(0, viewports);
  commandBuffer.setScissor(0, scissors);
  commandBuffer.draw(3, 1, 0, 0);
  commandBuffer.endRenderPass();
  commandBuffer.end();
}

void Graphics::draw() {
  std::array<vk::Fence, 1> fences = {*inFlight};
  std::array<vk::Semaphore, 1> waitSemaphores = {*imageAvailable};
  std::array<vk::Semaphore, 1> signalSemaphores = {*renderFinished};
  std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  std::array<vk::CommandBuffer, 1> commandBuffers = {*commandBuffer};
  std::array<vk::SwapchainKHR, 1> swapchains = {*swapchain.handle};

  if (device.handle.waitForFences(fences, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
    throw std::runtime_error("Failed waiting for fence");
  }
  device.handle.resetFences(fences);
  auto [result, imageIndex] = swapchain.handle.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailable);

  std::array<unsigned int, 1> imageIndices = {imageIndex};
  commandBuffer.reset();
  recordCommandBuffer(imageIndex);
  device.queue.submit(vk::SubmitInfo{}
                   .setWaitSemaphores(waitSemaphores)
                   .setWaitDstStageMask(waitStages)
                   .setCommandBuffers(commandBuffers)
                   .setSignalSemaphores(signalSemaphores),
               *inFlight);
  if (device.queue.presentKHR(vk::PresentInfoKHR{}
                           .setWaitSemaphores(signalSemaphores)
                           .setSwapchains(swapchains)
                           .setImageIndices(imageIndices)) != vk::Result::eSuccess) {
    throw std::runtime_error("Failed presentation");
  }
}
