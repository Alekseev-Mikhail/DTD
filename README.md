# Dummy 3D (DTD)
DTD is a simple 3D rendering library based on OpenGL and written in C

## Unit testing
Before running tests it is necessary to generate test runners and mocks. For that Ruby must be installed. <br><br>
Working directory for scripts below: **/tests** <br>
Generate test runners: <br>
```shell
ruby auto/generate_test_runner.rb test/test_list.c gen/test_list.c
```
Generate mocks: <br>
```shell
ruby auto/cmock.rb -o"config/CMockConfig.yml" ../src/utility/istdlib.h 
```

## Examples
### Simple cube
Program arguments: _shader_file_name(vararg)_

## Third-party libraries
- GLFW
- GLAD
- Unity
- CMock