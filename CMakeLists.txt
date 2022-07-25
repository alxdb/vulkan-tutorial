cmake_minimum_required(VERSION 3.22)
project(vulkan_tutorial)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(glfw3 3.3 REQUIRED)

add_compile_definitions(VKFW_NO_STRUCT_CONSTRUCTORS VULKAN_HPP_NO_CONSTRUCTORS)

add_subdirectory(lib)
add_subdirectory(src)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Vulkan::Vulkan glfw ${PROJECT_NAME})
