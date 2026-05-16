#ifndef WINDOW_H
#define WINDOW_H
#define GLFW_INCLUDE_NONE
#include "camera.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

typedef struct {
    const char *filename;
    const GLchar *source;
    GLenum type;
} Shader;

typedef struct {
    const char *shaderDirectory;
    Shader *shaders;
    size_t shaderCount;
    char **shaderFilenames;
} ShaderRegister;

typedef struct {
    GLFWwindow *id;
    GLuint _shaderProgram;
    size_t width, height;
    Camera *camera;
    ShaderRegister *shaderRegister;
} WindowData;

WindowData *win_init(int width, int height, const char *title);

void win_registerShaders(WindowData *win);

void win_compileShaders(WindowData *win);

void win_startRenderCycle(const WindowData *win);

void win_dispose(WindowData *win);

void win_disposeShaderRegister(WindowData *win);

void win_disposeAndAbort(WindowData *win);

#endif //WINDOW_H
