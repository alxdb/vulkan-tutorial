#pragma once

template <std::ranges::contiguous_range R>
HostBuffer<R>::HostBuffer(const vk::raii::Device &device,
                          const vk::raii::PhysicalDevice &physicalDevice,
                          const R &data,
                          vk::BufferUsageFlags usage)
    : data(data),
      Buffer(device,
             physicalDevice,
             sizeof(std::ranges::range_value_t<R>) * std::ranges::size(data),
             usage,
             vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible) {}

template <std::ranges::contiguous_range R>
void HostBuffer<R>::copyData() const {
  void *mappedMemory = this->memory.mapMemory(0, this->size);
  memcpy(mappedMemory, std::ranges::data(this->data), this->size);
  this->memory.unmapMemory();
}

template <typename T>
DynamicHostBuffer<T>::DynamicHostBuffer(const vk::raii::Device &device,
                                        const vk::raii::PhysicalDevice &physicalDevice,
                                        vk::BufferUsageFlags usage)
    : Buffer(device,
             physicalDevice,
             sizeof(T),
             usage,
             vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible) {}

template <typename T>
void DynamicHostBuffer<T>::copyData(const T &data) const {
  void *mappedMemory = this->memory.mapMemory(0, this->size);
  memcpy(mappedMemory, &data, this->size);
  this->memory.unmapMemory();
}

template <std::ranges::contiguous_range R>
StagedBuffer<R>::StagedBuffer(const vk::raii::Device &device,
                              const vk::raii::PhysicalDevice &physicalDevice,
                              const R &data,
                              vk::BufferUsageFlags usage)
    : stagingBuffer(device, physicalDevice, data, vk::BufferUsageFlagBits::eTransferSrc),
      deviceBuffer(device,
                   physicalDevice,
                   stagingBuffer.size,
                   vk::BufferUsageFlagBits::eTransferDst | usage,
                   vk::MemoryPropertyFlagBits::eDeviceLocal) {}

template <std::ranges::contiguous_range R>
void StagedBuffer<R>::copyData(const vk::raii::Device &device,
                               const vk::raii::CommandPool &commandPool,
                               const vk::raii::Queue &queue) const {
  stagingBuffer.copyData();

  auto commandBuffer = std::move(device.allocateCommandBuffers({
      .commandPool = *commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1,
  })[0]);

  commandBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandBuffer.copyBuffer(*stagingBuffer.buffer, *deviceBuffer.buffer, {{.size = stagingBuffer.size}});
  commandBuffer.end();

  auto commandBuffers = {*commandBuffer};
  queue.submit({vk::SubmitInfo{}.setCommandBuffers(commandBuffers)});
  queue.waitIdle();
}
