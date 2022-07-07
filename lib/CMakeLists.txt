add_library(vkfw INTERFACE)
target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
target_precompile_headers(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include/vkfw/vkfw.hpp)
