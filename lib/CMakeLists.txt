find_package(Vulkan REQUIRED COMPONENTS glslc)

add_subdirectory(glfw)

add_library(vkfw INTERFACE)
target_link_libraries(vkfw INTERFACE glfw Vulkan::Vulkan)
target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
