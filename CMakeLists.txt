cmake_minimum_required(VERSION 3.30)
project(simple_cube C)

set(CMAKE_C_STANDARD 11)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/glfw)
include_directories(SYSTEM src)
include_directories(SYSTEM third-party/glad/include)
include_directories(SYSTEM third-party/unity)
include_directories(SYSTEM third-party/cmock)

add_executable(simple_cube
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
        src/ecs/ecs.c
        src/ecs/ecs.h
        src/utility/list.c
        src/utility/list.h
        src/utility/istdlib.c
        src/utility/istdlib.h
)
target_link_libraries(simple_cube glfw)


# TEST EXECUTABLES

# list.c without mocking
add_executable(test_list
        src/utility/list.c

        tests/test/test_list.c
        tests/gen/test_list.c

        tests/gen/mocks/mock_istdlib.c

        third-party/unity/unity.c
        third-party/cmock/cmock.c
)
target_compile_definitions(test_list PRIVATE UNITY_OUTPUT_COLOR)
