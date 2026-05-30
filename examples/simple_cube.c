#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/window.h"
#include "../src/math/rad.h"
#include "../src/utility/exception.h"
#include "../src/utility/log.h"

static const char DEFAULT_ARGUMENTS_NUMBER = 1;

void intiShaderRegister(WindowData *win, int argc, char **argv);

void registerAndCompileShaders(WindowData *win, char **argv);

bool getExeDirectoryPath(char **exeDirPath, const char *exePath);

bool setDirectoryPath(WindowData *win, const char **directoryPath, const char *pathToOrigin, const char *relativePath);

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    glog(level, format, "main", args);
    va_end(args);
}

int main(const int argc, char **argv) {
    llog(INFO, "Initializing window");
    WindowData *win = win_init(1000, 700, "Hiya, OpenGL!");

    if (argc < DEFAULT_ARGUMENTS_NUMBER) {
        llog(ERROR, "Not enough arguments");
        win_disposeAndAbort(win);
    }

    if (argc < DEFAULT_ARGUMENTS_NUMBER + 1) {
        llog(WARN, "No shaders specified");
    } else {
        llog(INFO, "Initializing shader register");
        intiShaderRegister(win, argc, argv);
        registerAndCompileShaders(win, argv);
    }

    cam_setPrefs(win->camera, toRad(75), 0.1f, 100.0f);
    cam_move(win->camera, -3, 3, -3);
    cam_rotate(win->camera, toRad(-38.0f), toRad(-45.0f), 0);

    llog(INFO, "Starting render cycle");
    win_startRenderCycle(win);

    llog(INFO, "Shutting down application");
    win_dispose(win);
    llog(INFO, "Application was shut down properly");
    return 0;
}

void intiShaderRegister(WindowData *const win, const int argc, char **argv) {
    if (true == IS_NULL(win) || true == IS_NULL(argv)) {
        llog(FATAL, "Cannot create new shader register");
        win_disposeAndAbort(win);
    }

    win->shaderRegister = malloc(sizeof(ShaderRegister));

    win->shaderRegister->shaderCount = argc - 1;
    llog(DEBUG, "Shader count: %zu", win->shaderRegister->shaderCount);
    if (argc < 1 + win->shaderRegister->shaderCount) {
        llog(ERROR, "Not enough shader filenames");
        abort();
    }
    win->shaderRegister->shaderFilenames = malloc(win->shaderRegister->shaderCount * sizeof(char *));
    for (int i = 0; i < win->shaderRegister->shaderCount; i++) {
        win->shaderRegister->shaderFilenames[i] = argv[DEFAULT_ARGUMENTS_NUMBER + i];
    }

    win->shaderRegister->shaders = calloc(win->shaderRegister->shaderCount, sizeof(Shader));
}

void registerAndCompileShaders(WindowData *const win, char **argv) {
    llog(DEBUG, "Getting program arguments");
    char *exeDirPath = NULL;
    if (false == getExeDirectoryPath(&exeDirPath, argv[0])) {
        llog(FATAL, "Failed to get path, where executable is located");
        win_disposeAndAbort(win);
    }
    llog(DEBUG, "Working directory: %s", exeDirPath);

    if (false == setDirectoryPath(win, &win->shaderRegister->shaderDirectory, exeDirPath, "resources/shaders/")) {
        llog(FATAL, "Cannot get shader directory path");
        win_disposeAndAbort(win);
    }
    llog(DEBUG, "Shaders directory: %s", win->shaderRegister->shaderDirectory);
    free(exeDirPath);

    llog(INFO, "Registering and compiling shaders");
    llog(DEBUG, "Registering shaders");
    win_registerShaders(win);
    llog(DEBUG, "Compiling shaders");
    win_compileShaders(win);
    llog(INFO, "Disposing shader register");
    win_disposeShaderRegister(win);
}

bool getExeDirectoryPath(char **const exeDirPath, const char *const exePath) {
    if (exeDirPath == NULL || exePath == NULL) {
        llog(ERROR, "Null pointer exception");
        return false;
    }

    const int exePathLength = (int)strlen(exePath);
    unsigned int exeNameLength = 0;
    for (int i = exePathLength - 1; i >= 0; i--) {
        if (exePath[i] == '/') break;
        exeNameLength++;
    }

    const unsigned int exeDirPathBufferSize = (exePathLength - exeNameLength + 1) * sizeof(char);
    *exeDirPath = malloc(exeDirPathBufferSize);
    snprintf(*exeDirPath, exeDirPathBufferSize, "%s", exePath);
    return true;
}

bool setDirectoryPath(WindowData *const win, const char **const directoryPath, const char *const pathToOrigin, const char *const relativePath) {
    if (true == IS_NULL(win) || true == IS_NULL(directoryPath) || true == IS_NULL(pathToOrigin) || true == IS_NULL(relativePath)) {
        llog(FATAL, "Unable to set directory path based on relative path");
        win_disposeAndAbort(win);
    }

    const size_t directoryLength = strlen(pathToOrigin) + strlen(relativePath) + 1;
    *directoryPath = malloc(directoryLength * sizeof(char));
    const int result = snprintf((char *)*directoryPath, directoryLength, "%s%s", pathToOrigin, relativePath);
    if (result > 0 && result < directoryLength) return true;
    return false;
}
