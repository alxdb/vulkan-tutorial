#include <vector>

#include <SDL.h>

class Window {
	static int window_instances;
public:
	SDL_Window* window;
	Window(const char* window_name, int height, int width);
	Window(Window const&) = delete;
	void getInstanceExtensions(unsigned int* extension_count, const char** extension_names);
	~Window();
};
