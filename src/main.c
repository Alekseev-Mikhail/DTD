#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"
#include "math/rad.h"
#include "utility/log.h"

static size_t shaderCount;
static Shader *shaders;
static char **shaderFilenames;

void setShaderInfoFromArguments(int argc, char **argv);

static char *getShaderSource(WindowData *win, const char *filename);

static GLenum getShaderType(const char *filename);

void setupShaderCompiling(WindowData *win);

static void disposeShaders();

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    glog(level, format, "main", args);
    va_end(args);
}

int main(const int argc, char **argv) {
    llog(INFO, "Getting program arguments");
    if (argc < 3) {
        llog(ERROR, "Not enough arguments");
        abort();
    }
    resourceDirectory = argv[1];
    setShaderInfoFromArguments(argc, argv);

    llog(INFO, "Initializing window");
    WindowData *win = win_init(1000, 700, "Hiya, OpenGL!");

    llog(INFO, "Starting compiling shaders");
    setupShaderCompiling(win);
    win->envDisposer = disposeShaders;
    win_compileShaders(win, shaders, shaderCount);
    disposeShaders(shaders, shaderCount);
    win->envDisposer = NULL;

    cam_setPrefs(win->camera, toRad(75), 0.1f, 100.0f);
    cam_move(win->camera, -3, 3, -3);
    cam_rotate(win->camera, toRad(-38.0f), toRad(-45.0f), 0);

    llog(INFO, "Starting render cycle");
    win_startRenderCycle(win);

    llog(INFO, "Shutting down application");
    win_dispose(win);
    return 0;
}

void setShaderInfoFromArguments(const int argc, char **argv) {
    char *endP;
    shaderCount = strtol(argv[2], &endP, 10);
    if (*endP != '\0') {
        llog(ERROR, "Cannot read shader count as second argument");
        abort();
    }
    if (argc < 3 + shaderCount) {
        llog(ERROR, "Not enough shader filenames");
        abort();
    }
    shaderFilenames = malloc(shaderCount * sizeof(char *));
    for (int i = 0; i < shaderCount; i++) {
        shaderFilenames[i] = argv[3 + i];
    }
}

static char *getShaderSource(WindowData *const win, const char *const filename) {
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

static GLenum getShaderType(const char *const filename) {
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

void setupShaderCompiling(WindowData *const win) {
    shaders = malloc(shaderCount * sizeof(Shader));
    for (int i = 0; i < shaderCount; i++) {
        char *const filename = shaderFilenames[i];
        const GLenum type = getShaderType(filename);

        llog(INFO, "Getting shader source: %s", filename);

        if (type == GL_NONE) {
            llog(ERROR, "Unknown shader type for %s", filename);
            win_disposeAndAbort(win);
        }
        const Shader shader = {filename, getShaderSource(win, filename), type};
        shaders[i] = shader;
    }
}

static void disposeShaders() {
    llog(INFO, "Disposing shaders' sources");
    for (int i = 0; i < shaderCount; ++i) {
        free((void *) shaders[i].source);
    }
    free(shaderFilenames);
    free(shaders);
}
