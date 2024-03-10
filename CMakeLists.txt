cmake_minimum_required(VERSION 3.28)
project(wooboard)

# allows clangd to see external libs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)

# RTMIDI: midi library
add_subdirectory(external_libraries/rtmidi)

# GLFW: graphics library frame work

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external_libraries/glfw)

add_executable(wooboard
    main.cpp)

target_link_libraries(wooboard rtmidi glfw)
