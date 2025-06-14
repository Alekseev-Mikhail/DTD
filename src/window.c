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

WindowData *win_init(const int width, const int height, const char *title) {
    llog(INFO, "Initializing GLFW");
    if (!glfwInit()) {
        llog(ERROR, "Failed to initialize GLFW");
        abort();
    }

    llog(INFO, "Creating GLFW window");
    WindowData *win = malloc(sizeof(WindowData));
    win->id = glfwCreateWindow(width, height, title, NULL, NULL);
    win->width = width;
    win->height = height;
    win->camera = cam_allocate();
    win->camera->aspect = (float) width / (float) height;
    win->envDisposer = NULL;
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
    return win;
}

void win_compileShaders(WindowData *const win, const Shader shaders[], const size_t count) {
    GLuint shaderIds[count];

    for (int i = 0; i < count; i++) {
        Shader const shader = shaders[i];
        const GLuint shaderId = glCreateShader(shader.type);
        shaderIds[i] = shaderId;

        llog(INFO, "Compiling (%s) shader", shader.filename);
        glShaderSource(shaderId, 1, &shader.source, NULL);
        glCompileShader(shaderId);
        checkShaderCompilation(win, shaderId);
    }

    llog(INFO, "Creating a shader program");
    win->_shaderProgram = glCreateProgram();
    llog(INFO, "Attaching the shaders to the program");
    for (int i = 0; i < count; i++) {
        glAttachShader(win->_shaderProgram, shaderIds[i]);
    }
    llog(INFO, "Linking the shader program");
    glLinkProgram(win->_shaderProgram);
    checkShaderProgramLinking(win);

    for (int i = 0; i < count; i++) {
        const Shader shader = shaders[i];
        const GLuint shaderId = shaderIds[i];
        llog(INFO, "Detaching the (%s) shader from the program", shader.filename);
        glDetachShader(win->_shaderProgram, shaderId);
        llog(INFO, "Deleting the (%s) shader", shader.filename);
        glDeleteShader(shaderId);
    }
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
    if (win->envDisposer != NULL) win->envDisposer();
    glfwDestroyWindow(win->id);
    cam_dispose(win->camera);
    free(win);
    glfwTerminate();
    llog(INFO, "Application was shut down properly");
}
