#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
	glm::vec4 pos;
	glm::vec4 col;

	static const vk::VertexInputBindingDescription binding_description;
	static const std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;
};
