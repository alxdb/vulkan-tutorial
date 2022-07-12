add_subdirectory(shaders)

add_library(${PROJECT_NAME} graphics.cpp graphics.hpp device.cpp device.hpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vkfw shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
