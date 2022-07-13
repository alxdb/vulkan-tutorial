add_subdirectory(shaders)

add_library(
  ${PROJECT_NAME}
  base.cpp base.hpp
  device.cpp device.hpp
  graphics.cpp graphics.hpp
  swapchain.cpp swapchain.hpp
  pipeline.cpp pipeline.hpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC vkfw shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
