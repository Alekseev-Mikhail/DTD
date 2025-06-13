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
} WindowData;

extern const char *resourceDirectory;
extern const char *shaderDirectory;

void win_initWindow(WindowData *win, int width, int height, const char* title);

void win_startRenderCycle(const WindowData *win);

void win_disposeAndAbort(WindowData *win);

void win_dispose(WindowData *win);

#endif //WINDOW_H
