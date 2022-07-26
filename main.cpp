#include <iostream>

#include <graphics.hpp>

struct App {
  const vkfw::UniqueHandle<vkfw::Instance> vkfw = vkfw::initUnique();
  const vkfw::UniqueHandle<vkfw::Window> window =
      vkfw::createWindowUnique(1920, 1080, "vulkan tutorial", {.resizable = false});

  Graphics graphics;

  App() : graphics(*window) {}

  void main() {
    while (!window->shouldClose()) {
      vkfw::pollEvents();
      graphics.draw(*window);
    }
  }
};

int main() {
  App app;
  app.main();
}
