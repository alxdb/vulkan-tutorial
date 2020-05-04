#include <vector>

#include <SDL.h>
#include <vulkan/vulkan.hpp>

class Window {
	static int window_instances;
	SDL_Window* window = nullptr;
public:
	Window(const char* window_name, int height, int width);
	std::vector<const char*> get_instance_extensions() const;
	vk::UniqueSurfaceKHR create_surface(vk::UniqueInstance& instance);
	~Window();
};
