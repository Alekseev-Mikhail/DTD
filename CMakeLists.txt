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

# TESTS
add_executable(matrix-tests
        tests/matrix-tests.c
        tests/tool/unity.h
        tests/tool/unity_internals.h
        tests/tool/unity.c
        src/math/matrix.c
        src/math/matrix.h
        src/math/vector.c
        src/math/vector.h
)
add_executable(vector-tests
        tests/vector-tests.c
        tests/tool/unity.c
        tests/tool/unity.h
        tests/tool/unity_internals.h
        src/math/vector.c
        src/math/vector.h
)

target_link_libraries(dummy3d glfw)
