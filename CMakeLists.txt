cmake_minimum_required(VERSION 3.17)
project(FNode C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(raylib)
find_package(glfw3 3.3 REQUIRED)

#include_directories(src/external)

add_executable(FNode
        src/external/glad.h
        src/fnode.h
        src/fnode_shader_editor.c)

target_link_libraries(FNode raylib)
target_include_directories(FNode PUBLIC ${GLFW3_INCLUDE_DIRS})
