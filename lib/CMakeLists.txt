add_library(vkfw INTERFACE)
target_link_libraries(vkfw INTERFACE glfw Vulkan::Vulkan)
target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
target_precompile_headers(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include/vkfw/vkfw.hpp)
