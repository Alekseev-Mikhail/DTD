#include "window.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "utility/log.h"

const char *resourceDirectory = "";
const char *shaderDirectory = "shaders/";

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    putLogMessage(level, format, "window", args);
    va_end(args);
}

static void checkShaderProgramLinking(WindowData *const data) {
    GLint isLinked;
    glGetProgramiv(data->shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        int logLength;
        glGetProgramiv(data->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

        char *log = malloc(logLength * sizeof(char));
        glGetProgramInfoLog(data->shaderProgram, logLength, NULL, log);

        llog(ERROR, "Shader program failed to link: %s", log);
        free(log);
        win_disposeAndAbort(data);
    }
}

static void checkShaderCompilation(WindowData *const data, const GLuint shader) {
    GLint isCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        int logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        char *log = malloc(logLength * sizeof(char));
        glGetShaderInfoLog(shader, logLength, NULL, log);

        llog(ERROR, "Compilation failed: %s", log);
        free(log);
        win_disposeAndAbort(data);
    }
}

static GLchar *getShaderSource(WindowData *const data, const char *const filename) {
    const unsigned pathLength = strlen(resourceDirectory) + strlen(shaderDirectory) + strlen(filename) + 1;
    char path[pathLength];
    snprintf(path, pathLength, "%s%s%s", resourceDirectory, shaderDirectory, filename);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        llog(ERROR, "Failed to open a shader source. %s: %s", strerror(errno), path);
        win_disposeAndAbort(data);
    }

    fseek(file, 0, SEEK_END);
    const int size = (int) ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = malloc((size + 1) * sizeof(char));

    fread(source, sizeof(char), size, file);
    source[size] = '\0';

    fclose(file);
    return source;
}

static void compileShaders(WindowData *const data) {
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    llog(INFO, "Getting shader sources");
    const GLchar *const vertSource = getShaderSource(data, "s.vert");
    const GLchar *const fragSource = getShaderSource(data, "s.frag");

    llog(INFO, "Compiling a vertex shader");
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(data, vertexShader);

    llog(INFO, "Compiling a fragment shader");
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(data, fragmentShader);

    llog(INFO, "Creating a shader program");
    data->shaderProgram = glCreateProgram();
    llog(INFO, "Attaching the shaders to the program");
    glAttachShader(data->shaderProgram, vertexShader);
    glAttachShader(data->shaderProgram, fragmentShader);
    llog(INFO, "Linking the shader program");
    glLinkProgram(data->shaderProgram);
    checkShaderProgramLinking(data);

    llog(INFO, "Detaching the shaders from the program");
    glDetachShader(data->shaderProgram, vertexShader);
    glDetachShader(data->shaderProgram, fragmentShader);

    llog(INFO, "Deleting the shaders");
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free((void *) vertSource);
    free((void *) fragSource);
}

static void render(const WindowData *const data, const GLuint vertexBuffer) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(data->shaderProgram);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
}

void win_initWindow(WindowData *const data, const int width, const int height, const char *title) {
    llog(INFO, "Initializing GLFW");
    if (!glfwInit()) {
        llog(ERROR, "Failed to initialize GLFW");
        free(data);
        abort();
    }

    llog(INFO, "Creating GLFW window");
    data->window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (NULL == data->window) {
        llog(ERROR, "Failed to create GLFW window");
        glfwTerminate();
        free(data);
        abort();
    }

    int viewportWidth, viewportHeight;
    glfwMakeContextCurrent(data->window);
    gladLoadGL();
    glfwGetFramebufferSize(data->window, &viewportWidth, &viewportHeight);
    glViewport(0, 0, viewportWidth, viewportHeight);
    glfwSwapInterval(1);

    compileShaders(data);
}

void win_startupRenderCycle(const WindowData *const data) {
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    const GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glClearColor(0.302f, 0.286f, 0.631f, 1.0f);

    while (!glfwWindowShouldClose(data->window)) {
        render(data, vertexBuffer);
        glfwSwapBuffers(data->window);
        glfwPollEvents();
    }
}

void win_disposeAndAbort(WindowData *const data) {
    llog(ERROR, "Aborting with unknown exception");
    win_dispose(data);
    abort();
}

void win_dispose(WindowData *const data) {
    llog(INFO, "Application was shut down properly");
    glfwDestroyWindow(data->window);
    free(data);
    glfwTerminate();
}
