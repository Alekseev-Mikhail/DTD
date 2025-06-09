#ifndef WINDOW_H
#define WINDOW_H
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

typedef struct {
    GLFWwindow *window;
    GLuint shaderProgram;
} WindowData;

extern const char *resourceDirectory;
extern const char *shaderDirectory;

void win_initWindow(WindowData *data, int width, int height, const char* title);

void win_startupRenderCycle(const WindowData *data);

void win_disposeAndAbort(WindowData *data);

void win_dispose(WindowData *data);

#endif //WINDOW_H
