#include <vector>

#include <SDL.h>
#include <vulkan/vulkan.hpp>

class Window {
	static int window_instances;
	SDL_Window* window = nullptr;
public:
	Window(const char* window_name, int height, int width);
	std::vector<const char*> getInstanceExtensions();
	vk::UniqueSurfaceKHR createSurface(vk::UniqueInstance& instance);
	~Window();
};
