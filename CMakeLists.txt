cmake_minimum_required(VERSION 3.30)
project(dummy3d C)

set(CMAKE_C_STANDARD 11)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-source)
include_directories(SYSTEM glad/include)

add_executable(dummy3d
        src/main.c
        glad/src/glad.c
        src/window.c
        src/window.h
        src/utility/log.c
        src/utility/log.h
        src/math/matrix.c
        src/math/matrix.h
        src/math/vector.c
        src/math/vector.h
        src/camera.c
        src/camera.h
        src/math/rad.c
        src/math/rad.h
)

target_link_libraries(dummy3d glfw)
