#include <glm/gtx/rotate_vector.hpp>
#include "vertex.hh"

const vk::VertexInputBindingDescription Vertex::binding_description(
	0,
	sizeof(Vertex),
	vk::VertexInputRate::eVertex
);

const std::vector<vk::VertexInputAttributeDescription> Vertex::attribute_descriptions{{
	{0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, pos)},
	{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, col)}
}};

void Vertex::rotate(glm::vec3 axis, float angle) {
	pos = glm::rotate(pos, angle, axis);
}
