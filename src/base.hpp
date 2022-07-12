#pragma once

#ifndef VKFW_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS
#endif

#include <vkfw/vkfw.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Base {
    const vk::raii::Context context;
    const vk::raii::Instance instance;
    const vk::raii::SurfaceKHR surface;

    Base(const vkfw::Window& window);
};
