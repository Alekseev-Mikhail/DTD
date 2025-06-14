#ifndef WINDOW_H
#define WINDOW_H
#define GLFW_INCLUDE_NONE
#include "camera.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

typedef struct {
    GLFWwindow *id;
    GLuint _shaderProgram;
    size_t width, height;
    Camera *camera;

    void (*envDisposer)(void);
} WindowData;

typedef struct {
    const char *filename;
    const GLchar *source;
    GLenum type;
} Shader;

extern const char *resourceDirectory;
extern const char *shaderDirectory;

WindowData *win_init(int width, int height, const char *title);

void win_compileShaders(WindowData *win, const Shader shaders[], size_t count);

void win_startRenderCycle(const WindowData *win);

void win_disposeAndAbort(WindowData *win);

void win_dispose(WindowData *win);

#endif //WINDOW_H
