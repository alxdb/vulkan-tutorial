find_package(Vulkan REQUIRED COMPONENTS glslc)

if(USE_SYSTEM_GLFW)
  find_package(glfw3 REQUIRED)
else()
  add_subdirectory(glfw)
endif()

add_library(vkfw INTERFACE)
target_link_libraries(vkfw INTERFACE glfw Vulkan::Vulkan)
target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
