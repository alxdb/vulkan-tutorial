#include <iostream>

#include "vulkan_app.hh"

std::vector<Vertex> triangle = {
	{{+0.0f, -0.5f, +0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
	{{+0.5f, +0.5f, +0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
	{{-0.5f, +0.5f, +0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
};

class MyApp : public VulkanApp {
	virtual void update() override {
		for (Vertex& vertex : triangle) {
			vertex.rotate(glm::vec3(0.0, 0.0, 1.0), 0.01);
		}
		set_vertices(triangle);
	}
public:
	MyApp() : VulkanApp("vulkan tutorial", triangle) {};
};

int main() {
	MyApp app;
	app.run();
}
