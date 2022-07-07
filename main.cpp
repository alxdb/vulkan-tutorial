#include <graphics.hpp>

struct App {
  const vkfw::WindowHints windowHints = {
      .resizable = false,
  };
  vkfw::UniqueHandle<vkfw::Instance> vkfw = vkfw::initUnique();
  vkfw::UniqueHandle<vkfw::Window> window = vkfw::createWindowUnique(1920, 1080, "vulkan tutorial", windowHints);

  Graphics graphics;

  App() : graphics(*window) {}
};

int main(int argc, char *argv[]) {
  App app;
  while (!app.window->shouldClose()) {
    vkfw::pollEvents();
  }
  return 0;
}
