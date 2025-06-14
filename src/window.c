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

static void render(const WindowData *const win, const GLint mvpUniform, const GLuint vertexBuffer, const GLuint vertexColorBuffer) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

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
    win->camera->aspect = (float) height / (float) width;
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
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
    static const GLfloat vertexColors[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertexColorBuffer;
    glGenBuffers(1, &vertexColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);

    glClearColor(0.302f, 0.286f, 0.631f, 1.0f);

    const GLint mvpUniform = glGetUniformLocation(win->_shaderProgram, "mvp");

    while (!glfwWindowShouldClose(win->id)) {
        render(win, mvpUniform, vertexBuffer, vertexColorBuffer);
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
