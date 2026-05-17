cmake_minimum_required(VERSION 3.30)
project(dummy3d C)

set(CMAKE_C_STANDARD 11)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/glfw)
include_directories(SYSTEM third-party/glad/include)
include_directories(SYSTEM third-party/unity)

add_executable(dummy3d
        examples/simple_cube.c
        third-party/glad/src/glad.c
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
        src/utility/exception.h
)
target_link_libraries(dummy3d glfw)

add_executable(test_test
        third-party/unity/unity.c
        tests/test_test.c
        tests/gen/gentest_test.c
)
