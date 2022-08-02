find_package(Vulkan REQUIRED COMPONENTS glslc)

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
  add_subdirectory(glfw)
endif()

find_package(glm QUIET)
if(NOT glm_FOUND)
  add_subdirectory(glm)
endif()

add_library(vkfw INTERFACE)
target_link_libraries(vkfw INTERFACE glfw Vulkan::Vulkan)
target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
