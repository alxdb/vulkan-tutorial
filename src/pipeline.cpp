#include "pipeline.hpp"

#include "fragment_shader.h"
#include "vertex_shader.h"

vk::VertexInputBindingDescription Vertex::bindingDescription = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = vk::VertexInputRate::eVertex,
};

std::array<vk::VertexInputAttributeDescription, 2> Vertex::attributeDescriptions = {
    vk::VertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(Vertex, pos),
    },
    vk::VertexInputAttributeDescription{
        .location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(Vertex, color),
    },
};

vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device &device) {
  auto layoutBindings = {vk::DescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
  }};
  return device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{}.setBindings(layoutBindings));
}

vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device &device,
                                              const vk::raii::DescriptorSetLayout &descriptorSetLayout) {
  auto descriptorSetLayouts = {*descriptorSetLayout};
  return device.createPipelineLayout(vk::PipelineLayoutCreateInfo{}.setSetLayouts(descriptorSetLayouts));
}

vk::raii::RenderPass createRenderPass(const vk::Format &format, const vk::raii::Device &device) {
  auto attachments = {
      vk::AttachmentDescription{
          .format = format,
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = vk::ImageLayout::eUndefined,
          .finalLayout = vk::ImageLayout::ePresentSrcKHR,
      },
  };
  auto colorAttachmentReferences = {
      vk::AttachmentReference{
          .attachment = 0,
          .layout = vk::ImageLayout::eColorAttachmentOptimal,
      },
  };
  auto subpasses = {
      vk::SubpassDescription{
          .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      }
          .setColorAttachments(colorAttachmentReferences),
  };
  auto dependencies = {
      vk::SubpassDependency{
          .srcSubpass = VK_SUBPASS_EXTERNAL,
          .dstSubpass = 0,
          .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
          .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
          .srcAccessMask = vk::AccessFlagBits::eNone,
          .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
      },
  };

  return device.createRenderPass(
      vk::RenderPassCreateInfo{}.setAttachments(attachments).setSubpasses(subpasses).setDependencies(dependencies));
}

vk::raii::Pipeline createPipeline(const vk::raii::Device &device,
                                  const vk::raii::PipelineLayout &layout,
                                  const vk::raii::RenderPass &renderPass) {
  auto vertexShader = device.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(vertex_shader_code));
  auto fragmentShader = device.createShaderModule(vk::ShaderModuleCreateInfo{}.setCode(fragment_shader_code));
  auto shaderStages = {
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
  auto dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };
  auto vertexInputBindingDescriptions = {
      Vertex::bindingDescription,
  };
  auto dynamicState = vk::PipelineDynamicStateCreateInfo{}.setDynamicStates(dynamicStates);
  auto vertexInput = vk::PipelineVertexInputStateCreateInfo{}
                         .setVertexBindingDescriptions(vertexInputBindingDescriptions)
                         .setVertexAttributeDescriptions(Vertex::attributeDescriptions);
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
      .frontFace = vk::FrontFace::eCounterClockwise,
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
          .layout = *layout,
          .renderPass = *renderPass,
          .subpass = 0,
      }
          .setStages(shaderStages);
  return device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
}

Pipeline::Pipeline(const vk::Format &format, const vk::raii::Device &device)
    : descriptorSetLayout(createDescriptorSetLayout(device)),
      pipelineLayout(createPipelineLayout(device, descriptorSetLayout)),
      renderPass(createRenderPass(format, device)),
      handle(createPipeline(device, pipelineLayout, renderPass)) {}
