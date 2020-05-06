#include "vulkan_app.hh"

const std::vector<Vertex> triangle = {
	{{+0.0f, -0.5f, +0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
	{{+0.5f, +0.5f, +0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
	{{-0.5f, +0.5f, +0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
};

int main() {
	VulkanApp app("vulkan_tutorial", triangle);
	app.run();
}
