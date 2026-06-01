#include "window.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math/matrix.h"
#include "math/rad.h"
#include "utility/exception.h"
#include "utility/log.h"
#include <errno.h>

static GLenum getShaderType(WindowData *win, const char *filename);

static char *getShaderSource(WindowData *win, const char *filename);

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
    win->shaderRegister = NULL;
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

void win_registerShaders(WindowData *const win) {
    for (int i = 0; i < win->shaderRegister->shaderCount; i++) {
        char *const filename = win->shaderRegister->shaderFilenames[i];
        const GLenum type = getShaderType(win, filename);

        if (type == GL_NONE) {
            llog(FATAL, "Unknown shader type for %s", filename);
            win_disposeAndAbort(win);
        }
        const Shader shader = {filename, getShaderSource(win, filename), type};
        win->shaderRegister->shaders[i] = shader;

        llog(INFO, "Shader was registered: %s", filename);
    }
}

static GLenum getShaderType(WindowData *const win, const char *const filename) {
    if (true == IS_NULL(filename)) {
        llog(FATAL, "Shader file name is undefined");
        win_disposeAndAbort(win);
    }
    char temp[strlen(filename) + 1];
    strcpy(temp, filename);
    strtok(temp, ".");
    const char *const token = strtok(NULL, ".");
    if (token != NULL) {
        if (strcmp(token, "vert") == 0) {
            return GL_VERTEX_SHADER;
        }
        if (strcmp(token, "frag") == 0) {
            return GL_FRAGMENT_SHADER;
        }
        if (strcmp(token, "geom") == 0) {
            return GL_GEOMETRY_SHADER;
        }
        if (strcmp(token, "tesc") == 0) {
            return GL_TESS_CONTROL_SHADER;
        }
        if (strcmp(token, "tese") == 0) {
            return GL_TESS_EVALUATION_SHADER;
        }
    }
    return GL_NONE;
}

static char *getShaderSource(WindowData *const win, const char *const filename) {
    const unsigned pathLength = strlen(win->shaderRegister->shaderDirectory) + strlen(filename) + 1;
    char path[pathLength];
    snprintf(path, pathLength, "%s%s", win->shaderRegister->shaderDirectory, filename);
    llog(DEBUG, "Shader path: %s", path);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        llog(ERROR, "Failed to open a shader source. %s: %s", strerror(errno), path);
        win_disposeAndAbort(win);
    }

    fseek(file, 0, SEEK_END);
    const int size = (int) ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = malloc(size * sizeof(char));
    if (true == IS_NULL(source)) {
        fclose(file);
        llog(ERROR, "Failed to allocate memory for shader source");
        win_disposeAndAbort(win);
    }
    fread(source, sizeof(char), size, file);

    // why are there so much null terminators at the end of a shader file!!!??
    int actualSourceSize = 0;
    char *actualSource = NULL;
    for (; actualSourceSize < size; actualSourceSize++) {
        if ('\0' == source[actualSourceSize]) break;
    }
    actualSourceSize++;
    actualSource = realloc(source, actualSourceSize);
    if (NULL == actualSource) {
        llog(ERROR, "Cannot get actual shader source");
        return source;
    }

    // I have no freaking idea why there are garbage characters at the end of a shader file
    for (int i = actualSourceSize - 1; i >= 0; i--) {
        if (125 == actualSource[i]) break;
        if (32 > actualSource[i] && actualSource[i] != '\0') {
            actualSource[i] = 32;
        }
    }

    fclose(file);
    return actualSource;
}

void win_compileShaders(WindowData *const win) {
    const size_t count = win->shaderRegister->shaderCount;
    GLuint shaderIds[count];

    for (int i = 0; i < count; i++) {
        Shader const shader = win->shaderRegister->shaders[i];
        const GLuint shaderId = glCreateShader(shader.type);
        shaderIds[i] = shaderId;

        llog(INFO, "Compiling shader: %s", shader.filename);
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

    llog(INFO, "Detaching and deleting shaders");
    for (int i = 0; i < count; i++) {
        const Shader shader = win->shaderRegister->shaders[i];
        const GLuint shaderId = shaderIds[i];
        llog(DEBUG, "Detaching shader from the program: %s", shader.filename);
        glDetachShader(win->_shaderProgram, shaderId);
        llog(DEBUG, "Deleting shader: %s", shader.filename);
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

void win_dispose(WindowData *const win) {
    if (true == IS_NULL(win)) {
        llog(FATAL, "CRITICAL: window's pointer is NULL");
        abort();
    }
    if (NULL != win->shaderRegister) win_disposeShaderRegister(win);
    glfwDestroyWindow(win->id);
    cam_dispose(win->camera);
    free(win);
    glfwTerminate();
}

void win_disposeShaderRegister(WindowData *const win) {
    llog(INFO, "Disposing shaders' sources");
    for (int i = 0; i < win->shaderRegister->shaderCount; ++i) {
        void *ptr = (void *) win->shaderRegister->shaders[i].source;
        free(ptr);
    }
    free(win->shaderRegister->shaderFilenames);
    free(win->shaderRegister->shaders);
    free((void *)win->shaderRegister->shaderDirectory);
    free(win->shaderRegister);
    win->shaderRegister = NULL;
}

void win_disposeAndAbort(WindowData *const win) {
    llog(FATAL, "Aborting with unknown exception");
    win_dispose(win);
    llog(WARN, "Application was shut down due to exception");
    abort();
}
