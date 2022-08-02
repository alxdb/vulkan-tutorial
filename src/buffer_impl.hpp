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
