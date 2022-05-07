cmake_minimum_required(VERSION 3.23)
project(vulkan_tutorial)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(glfw3 3.3 REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${SHADER_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

file(GLOB SHADERS shaders/*.vert shaders/*.frag)

find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)
foreach(GLSL IN LISTS SHADERS)
  get_filename_component(FILENAME ${GLSL} NAME)
  set(SPIRV ${SHADER_BINARY_DIR}/${FILENAME}.spv)
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSLC} -o ${SPIRV} ${GLSL}
    COMMENT "Compiling ${FILENAME}"
    DEPENDS ${SHADER_BINARY_DIR}
  )
  list(APPEND SHADER_BINARIES ${SPIRV})
endforeach()

add_custom_target(spirv ALL DEPENDS ${SHADER_BINARIES})

add_executable(main main.cpp ${SHADERS})
add_dependencies(main spirv)
