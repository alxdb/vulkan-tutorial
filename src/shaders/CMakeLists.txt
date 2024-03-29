find_program(glslc NAMES glslc HINTS Vulkan::glslc)

function(ADD_SHADER SHADER SHADER_SOURCE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.h
    COMMAND ${glslc} -mfmt=num
      ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.txt
    COMMAND ${CMAKE_COMMAND}
      -DSHADER_H=${CMAKE_CURRENT_SOURCE_DIR}/shader.h.in
      -DSHADER=${SHADER}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/shader.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shader.h.in
  )
  add_library(${SHADER} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.h)
  target_include_directories(${SHADER} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

add_shader(vertex_shader shader.vert)
add_shader(fragment_shader shader.frag)
add_library(shaders INTERFACE)
target_link_libraries(shaders INTERFACE vertex_shader fragment_shader)
