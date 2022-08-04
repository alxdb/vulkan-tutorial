#pragma once

vk::raii::DeviceMemory allocateMemory(const vk::raii::Device &,
                                      const vk::raii::PhysicalDevice &,
                                      const vk::raii::Buffer &,
                                      vk::MemoryPropertyFlags);

template <typename T>
Buffer<T>::Buffer(const vk::raii::Device &device,
                  const vk::raii::PhysicalDevice &physicalDevice,
                  size_t size,
                  vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags memoryProperties)
    : size(size),
      buffer(device.createBuffer({
          .size = size,
          .usage = usage,
          .sharingMode = vk::SharingMode::eExclusive,
      })),
      memory(allocateMemory(device, physicalDevice, buffer, memoryProperties)) {
  buffer.bindMemory(*memory, 0);
}

template <typename T>
HostBuffer<T>::HostBuffer(const vk::raii::Device &device,
                          const vk::raii::PhysicalDevice &physicalDevice,
                          const std::vector<T> &data,
                          vk::BufferUsageFlags usage)
    : data(data),
      Buffer<T>(device,
                physicalDevice,
                sizeof(T) * data.size(),
                usage,
                vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible) {}

template <typename T>
void HostBuffer<T>::copyData() const {
  void *mappedMemory = this->memory.mapMemory(0, this->size);
  memcpy(mappedMemory, this->data.data(), this->size);
  this->memory.unmapMemory();
}

template <typename T>
DynamicHostBuffer<T>::DynamicHostBuffer(const vk::raii::Device &device,
                                        const vk::raii::PhysicalDevice &physicalDevice,
                                        vk::BufferUsageFlags usage)
    : Buffer<T>(device,
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

template <typename T>
StagedBuffer<T>::StagedBuffer(const vk::raii::Device &device,
                              const vk::raii::PhysicalDevice &physicalDevice,
                              const std::vector<T> &data,
                              vk::BufferUsageFlags usage)
    : stagingBuffer(device, physicalDevice, data, vk::BufferUsageFlagBits::eTransferSrc),
      deviceBuffer(device,
                   physicalDevice,
                   stagingBuffer.size,
                   vk::BufferUsageFlagBits::eTransferDst | usage,
                   vk::MemoryPropertyFlagBits::eDeviceLocal) {}

template <typename T>
void StagedBuffer<T>::copyData(const vk::raii::Device &device,
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
