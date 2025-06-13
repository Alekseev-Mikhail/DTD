#include "window.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "math/matrix.h"
#include "math/rad.h"
#include "utility/log.h"

const char *resourceDirectory = "";
const char *shaderDirectory = "shaders/";

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    glog(level, format, "window", args);
    va_end(args);
}

static void checkShaderProgramLinking(WindowData *const win) {
    GLint isLinked;
    glGetProgramiv(win->_shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        int logLength;
        glGetProgramiv(win->_shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

        char *log = malloc(logLength * sizeof(char));
        glGetProgramInfoLog(win->_shaderProgram, logLength, NULL, log);

        llog(ERROR, "Shader program failed to link: %s", log);
        free(log);
        win_disposeAndAbort(win);
    }
}

static void checkShaderCompilation(WindowData *const win, const GLuint shader) {
    GLint isCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        int logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        char *log = malloc(logLength * sizeof(char));
        glGetShaderInfoLog(shader, logLength, NULL, log);

        llog(ERROR, "Compilation failed: %s", log);
        free(log);
        win_disposeAndAbort(win);
    }
}

static GLchar *getShaderSource(WindowData *const win, const char *const filename) {
    const unsigned pathLength = strlen(resourceDirectory) + strlen(shaderDirectory) + strlen(filename) + 1;
    char path[pathLength];
    snprintf(path, pathLength, "%s%s%s", resourceDirectory, shaderDirectory, filename);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        llog(ERROR, "Failed to open a shader source. %s: %s", strerror(errno), path);
        win_disposeAndAbort(win);
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

static void compileShaders(WindowData *const win) {
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    llog(INFO, "Getting shader sources");
    const GLchar *const vertSource = getShaderSource(win, "s.vert");
    const GLchar *const fragSource = getShaderSource(win, "s.frag");

    llog(INFO, "Compiling a vertex shader");
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(win, vertexShader);

    llog(INFO, "Compiling a fragment shader");
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(win, fragmentShader);

    llog(INFO, "Creating a shader program");
    win->_shaderProgram = glCreateProgram();
    llog(INFO, "Attaching the shaders to the program");
    glAttachShader(win->_shaderProgram, vertexShader);
    glAttachShader(win->_shaderProgram, fragmentShader);
    llog(INFO, "Linking the shader program");
    glLinkProgram(win->_shaderProgram);
    checkShaderProgramLinking(win);

    llog(INFO, "Detaching the shaders from the program");
    glDetachShader(win->_shaderProgram, vertexShader);
    glDetachShader(win->_shaderProgram, fragmentShader);

    llog(INFO, "Deleting the shaders");
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free((void *) vertSource);
    free((void *) fragSource);
}

static void render(const WindowData *const win, const GLint mvpUniform, const GLuint vertexBuffer) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(win->_shaderProgram);

    cam_updateMatrices(win->camera);
    Matrix4f mvp = {};
    Matrix4f m = {};
    Matrix4f p = {};
    Matrix4f r = {};
    const Vector3f pos = {0.0f, 0.0f, 0.0f};
    const Vector3f rot = {toRad(0.0f), toRad(0.0f), toRad(0.0f)};
    mat_translation(&p, &pos);
    mat_rotation(&r, &rot);
    mat_multMat4f(&p, &r, &m);
    mat_multMat4f(win->camera->vp, &m, &mvp);

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, mvp.t[0]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
    glDisableVertexAttribArray(0);
}

void win_initWindow(WindowData *const win, const int width, const int height, const char *title) {
    llog(INFO, "Initializing GLFW");
    if (!glfwInit()) {
        llog(ERROR, "Failed to initialize GLFW");
        free(win);
        abort();
    }

    llog(INFO, "Creating GLFW window");
    win->id = glfwCreateWindow(width, height, title, NULL, NULL);
    win->width = width;
    win->height = height;
    win->camera = cam_allocate();
    win->camera->aspect = (float) width / (float) height;
    if (NULL == win->id) {
        llog(ERROR, "Failed to create GLFW window");
        glfwTerminate();
        free(win);
        abort();
    }

    int viewportWidth, viewportHeight;
    glfwMakeContextCurrent(win->id);
    gladLoadGL();
    glfwGetFramebufferSize(win->id, &viewportWidth, &viewportHeight);
    glViewport(0, 0, viewportWidth, viewportHeight);
    glfwSwapInterval(1);

    compileShaders(win);
}

void win_startRenderCycle(const WindowData *const win) {
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    const GLfloat vertices[] = {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glClearColor(0.302f, 0.286f, 0.631f, 1.0f);

    const GLint mvpUniform = glGetUniformLocation(win->_shaderProgram, "mvp");

    while (!glfwWindowShouldClose(win->id)) {
        render(win, mvpUniform, vertexBuffer);
        glfwSwapBuffers(win->id);
        glfwPollEvents();
    }
}

void win_disposeAndAbort(WindowData *const win) {
    llog(ERROR, "Aborting with unknown exception");
    win_dispose(win);
    abort();
}

void win_dispose(WindowData *const win) {
    llog(INFO, "Application was shut down properly");
    glfwDestroyWindow(win->id);
    cam_dispose(win->camera);
    free(win);
    glfwTerminate();
}
