#include "graphics.hpp"

#include <iterator>
#include <ranges>
#include <unordered_set>

#include "fragment_shader.h"
#include "vertex_shader.h"

vk::raii::RenderPass Graphics::createRenderPass() const {
  std::array<vk::AttachmentDescription, 1> attachments = {
      vk::AttachmentDescription{
          .format = swapchain.details.format,
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = vk::ImageLayout::eUndefined,
          .finalLayout = vk::ImageLayout::ePresentSrcKHR,
      },
  };
  std::array<vk::AttachmentReference, 1> colorAttachmentReferences = {
      vk::AttachmentReference{
          .attachment = 0,
          .layout = vk::ImageLayout::eColorAttachmentOptimal,
      },
  };
  std::array<vk::SubpassDescription, 1> subpasses = {
      vk::SubpassDescription{
          .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      }
          .setColorAttachments(colorAttachmentReferences),
  };
  std::array<vk::SubpassDependency, 1> dependencies = {
      vk::SubpassDependency{
          .srcSubpass = VK_SUBPASS_EXTERNAL,
          .dstSubpass = 0,
          .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
          .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
          .srcAccessMask = vk::AccessFlagBits::eNone,
          .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
      },
  };

  auto createInfo =
      vk::RenderPassCreateInfo{}.setAttachments(attachments).setSubpasses(subpasses).setDependencies(dependencies);
  return device.handle.createRenderPass(createInfo);
}

vk::raii::Pipeline Graphics::createPipeline() const {
  auto vertexShader = device.handle.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(vertex_shader_code));
  auto fragmentShader = device.handle.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(fragment_shader_code));
  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
      vk::PipelineShaderStageCreateInfo{
          .stage = vk::ShaderStageFlagBits::eVertex,
          .module = *vertexShader,
          .pName = "main",
      },
      vk::PipelineShaderStageCreateInfo{
          .stage = vk::ShaderStageFlagBits::eFragment,
          .module = *fragmentShader,
          .pName = "main",
      },
  };

  // fixed functions
  std::array<vk::DynamicState, 2> dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };
  auto dynamicState = vk::PipelineDynamicStateCreateInfo{}.setDynamicStates(dynamicStates);
  auto vertexInput = vk::PipelineVertexInputStateCreateInfo{};
  auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = false,
  };
  auto viewportState = vk::PipelineViewportStateCreateInfo{
      .viewportCount = 1,
      .scissorCount = 1,
  };
  auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
      .depthClampEnable = false,
      .rasterizerDiscardEnable = false,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = false,
      .lineWidth = 1.0f,
  };
  auto multisampling = vk::PipelineMultisampleStateCreateInfo{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = false,
  };
  auto colorBlendAttachment = std::array<vk::PipelineColorBlendAttachmentState, 1>{
      vk::PipelineColorBlendAttachmentState{
          .blendEnable = false,
          .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
      },
  };
  auto colorBlending =
      vk::PipelineColorBlendStateCreateInfo{
          .logicOpEnable = false,
      }
          .setAttachments(colorBlendAttachment);

  auto graphicsPipelineCreateInfo =
      vk::GraphicsPipelineCreateInfo{
          .pVertexInputState = &vertexInput,
          .pInputAssemblyState = &inputAssembly,
          .pViewportState = &viewportState,
          .pRasterizationState = &rasterizer,
          .pMultisampleState = &multisampling,
          .pColorBlendState = &colorBlending,
          .pDynamicState = &dynamicState,
          .layout = *pipelineLayout,
          .renderPass = *renderPass,
          .subpass = 0,
      }
          .setStages(shaderStages);
  return device.handle.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
}

std::vector<vk::raii::Framebuffer> Graphics::createFramebuffers() const {
  std::vector<vk::raii::Framebuffer> result;
  result.reserve(swapchain.images.size());

  std::ranges::transform(swapchain.images, std::back_inserter(result), [&](const auto &view) {
    std::array<vk::ImageView, 1> attachments = {*view};
    auto createInfo =
        vk::FramebufferCreateInfo{
            .renderPass = *renderPass,
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
          .renderPass = *renderPass,
          .framebuffer = *framebuffers[framebuffer_index],
          .renderArea =
              {
                  .offset = {0, 0},
                  .extent = swapchain.details.extent,
              },
      }
          .setClearValues(clearValues),
      vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
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
