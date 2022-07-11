#include <graphics.hpp>

struct App {
  const vkfw::WindowHints windowHints = {
      .resizable = false,
  };
  vkfw::UniqueHandle<vkfw::Instance> vkfw = vkfw::initUnique();
  vkfw::UniqueHandle<vkfw::Window> window = vkfw::createWindowUnique(1920, 1080, "vulkan tutorial", windowHints);

  Graphics graphics;

  App() : graphics(*window) {}

  void mainLoop() {
    while (!window->shouldClose()) {
      vkfw::pollEvents();
      graphics.draw();
    }
    graphics.waitIdle();
  }
};

int main(int argc, char *argv[]) {
  App app;
  app.mainLoop();
  return 0;
}
