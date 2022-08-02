add_subdirectory(shaders)

add_library(
  ${PROJECT_NAME}
  base.cpp base.hpp
  device.cpp device.hpp
  frame.cpp frame.hpp
  graphics.cpp graphics.hpp
  pipeline.cpp pipeline.hpp
  swapchain.cpp swapchain.hpp
  buffer.cpp buffer.hpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC vkfw shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLM_INCLUDE_DIRS})
