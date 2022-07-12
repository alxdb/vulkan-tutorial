#include "graphics.hpp"

#include <iterator>
#include <ranges>
#include <unordered_set>

#include "fragment_shader.h"
#include "vertex_shader.h"

const std::array<const char *, 1> REQUIRED_DEVICE_EXTENSION_NAMES = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const std::array<const char *, 1> REQUIRED_LAYER_NAMES = {
#ifndef NDEBUG
    "VK_LAYER_KHRONOS_validation",
#endif
};

vk::raii::Instance Graphics::createInstance() const {
  auto requiredExtensions = vkfw::getRequiredInstanceExtensions();
  auto applicationInfo = vk::ApplicationInfo{.apiVersion = VK_API_VERSION_1_1};

  return {
      context,
      vk::InstanceCreateInfo{.pApplicationInfo = &applicationInfo}
          .setPEnabledExtensionNames(requiredExtensions)
          .setPEnabledLayerNames(REQUIRED_LAYER_NAMES),
  };
}

vk::raii::PhysicalDevice Graphics::pickPhysicalDevice() {
  auto isSuitable = [&](const vk::raii::PhysicalDevice &pd) {
    auto deviceExtensionsProperties = pd.enumerateDeviceExtensionProperties();

    std::unordered_set<std::string> availableExtensionNames;
    std::ranges::transform(deviceExtensionsProperties,
                           std::inserter(availableExtensionNames, availableExtensionNames.begin()),
                           [](auto props) { return props.extensionName; });
    for (auto extensionName : REQUIRED_DEVICE_EXTENSION_NAMES) {
      if (!availableExtensionNames.contains(extensionName)) {
        return false;
      }
    }
    swapchainSupportDetails = {pd, surface};
    if (swapchainSupportDetails.formats.empty() || swapchainSupportDetails.presentModes.empty()) {
      return false;
    }

    auto queueFamilies = pd.getQueueFamilyProperties();
    auto queueFamily = std::ranges::find_if(queueFamilies, [](const auto &queueFamily) {
      return (bool)(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
    });
    if (queueFamily == queueFamilies.end()) {
      return false;
    }

    queueFamilyIndex = std::distance(queueFamilies.begin(), queueFamily);
    return static_cast<bool>(pd.getSurfaceSupportKHR(queueFamilyIndex, *surface));
  };

  auto physicalDevices = instance.enumeratePhysicalDevices();
  if (auto result = std::ranges::find_if(physicalDevices, isSuitable); result != physicalDevices.end()) {
    return *result;
  } else {
    throw std::runtime_error("No suitabled devices");
  }
}

vk::raii::Device Graphics::createDevice() const {
  std::array<float, 1> queuePriorities{1.0};
  auto queueCreateInfos = {
      vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = queueFamilyIndex,
          .queueCount = 1,
      }
          .setQueuePriorities(queuePriorities),
  };
  return {
      physicalDevice,
      vk::DeviceCreateInfo{}
          .setPEnabledExtensionNames(REQUIRED_DEVICE_EXTENSION_NAMES)
          .setQueueCreateInfos(queueCreateInfos),
  };
}

vk::raii::SwapchainKHR Graphics::createSwapchain(const vkfw::Window &window) {
  if (auto it = std::ranges::find(swapchainSupportDetails.formats,
                                  vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear});
      it != swapchainSupportDetails.formats.end()) {
    surfaceFormat = *it;
  } else {
    surfaceFormat = swapchainSupportDetails.formats.back();
  }

  vk::PresentModeKHR presentMode;
  if (auto it = std::ranges::find(swapchainSupportDetails.presentModes, vk::PresentModeKHR::eMailbox);
      it != swapchainSupportDetails.presentModes.end()) {
    presentMode = *it;
  } else {
    presentMode = vk::PresentModeKHR::eFifo;
  }

  if (swapchainSupportDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    swapExtent = swapchainSupportDetails.capabilities.currentExtent;
  } else {
    auto [realWidth, realHeight] = static_cast<std::tuple<uint32_t, uint32_t>>(window.getFramebufferSize());
    swapExtent = vk::Extent2D{
        std::clamp(realWidth,
                   swapchainSupportDetails.capabilities.minImageExtent.width,
                   swapchainSupportDetails.capabilities.maxImageExtent.width),
        std::clamp(realHeight,
                   swapchainSupportDetails.capabilities.minImageExtent.height,
                   swapchainSupportDetails.capabilities.maxImageExtent.height),
    };
  }

  uint32_t minImageCount;
  if (swapchainSupportDetails.capabilities.maxImageCount == 0) {
    minImageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
  } else {
    minImageCount = swapchainSupportDetails.capabilities.maxImageCount;
  }

  auto result = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR{
      .surface = *surface,
      .minImageCount = minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = swapExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = swapchainSupportDetails.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true,
      .oldSwapchain = swapchainCreated ? *swapchain : VK_NULL_HANDLE,
  });
  swapchainCreated = true;
  return result;
}

std::vector<vk::raii::ImageView> Graphics::createImageViews() const {
  std::vector<vk::raii::ImageView> result;
  result.reserve(swapchainImages.size());

  std::ranges::transform(swapchainImages, std::back_inserter(result), [&](const auto &image) {
    return device.createImageView(vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = surfaceFormat.format,
        .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    });
  });

  return result;
}

vk::raii::RenderPass Graphics::createRenderPass() const {
  std::array<vk::AttachmentDescription, 1> attachments = {
      vk::AttachmentDescription{
          .format = surfaceFormat.format,
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
  return device.createRenderPass(createInfo);
}

vk::raii::Pipeline Graphics::createPipeline() const {
  auto vertexShader = device.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(vertex_shader_code));
  auto fragmentShader = device.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(fragment_shader_code));
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
  return device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
}

std::vector<vk::raii::Framebuffer> Graphics::createFramebuffers() const {
  std::vector<vk::raii::Framebuffer> result;
  result.reserve(imageViews.size());

  std::ranges::transform(imageViews, std::back_inserter(result), [&](const auto &view) {
    std::array<vk::ImageView, 1> attachments = {*view};
    auto createInfo =
        vk::FramebufferCreateInfo{
            .renderPass = *renderPass,
            .width = swapExtent.width,
            .height = swapExtent.height,
            .layers = 1,
        }
            .setAttachments(attachments);
    return device.createFramebuffer(createInfo);
  });

  return result;
}

void Graphics::recordCommandBuffer(size_t framebuffer_index) const {
  std::array<vk::ClearValue, 1> clearValues = {{{{{{0.0f, 0.0f, 0.0f, 1.0f}}}}}};
  std::array<vk::Viewport, 1> viewports = {vk::Viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapExtent.width),
      .height = static_cast<float>(swapExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  }};
  std::array<vk::Rect2D, 1> scissors = {vk::Rect2D{
      .offset = {0, 0},
      .extent = swapExtent,
  }};

  commandBuffer.begin({});
  commandBuffer.beginRenderPass(
      vk::RenderPassBeginInfo{
          .renderPass = *renderPass,
          .framebuffer = *framebuffers[framebuffer_index],
          .renderArea =
              {
                  .offset = {0, 0},
                  .extent = swapExtent,
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
  std::array<vk::SwapchainKHR, 1> swapchains = {*swapchain};

  if (device.waitForFences(fences, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
    throw std::runtime_error("Failed waiting for fence");
  }
  device.resetFences(fences);
  auto [result, imageIndex] = swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailable);

  std::array<unsigned int, 1> imageIndices = {imageIndex};
  commandBuffer.reset();
  recordCommandBuffer(imageIndex);
  queue.submit(vk::SubmitInfo{}
                   .setWaitSemaphores(waitSemaphores)
                   .setWaitDstStageMask(waitStages)
                   .setCommandBuffers(commandBuffers)
                   .setSignalSemaphores(signalSemaphores),
               *inFlight);
  if (queue.presentKHR(vk::PresentInfoKHR{}
                           .setWaitSemaphores(signalSemaphores)
                           .setSwapchains(swapchains)
                           .setImageIndices(imageIndices)) != vk::Result::eSuccess) {
    throw std::runtime_error("Failed presentation");
  }
}
