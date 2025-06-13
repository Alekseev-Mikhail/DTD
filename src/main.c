#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "window.h"
#include "math/rad.h"
#include "utility/log.h"

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    glog(level, format, "main", args);
    va_end(args);
}

int main(const int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Directory for resources was not defined. Try pass a path as a program argument.");
        abort();
    }
    resourceDirectory = argv[1];

    llog(INFO, "Initializing window");
    WindowData *win = malloc(sizeof(WindowData));
    win_initWindow(win, 500, 500, "Hiya, OpenGL!");

    cam_setPrefs(win->camera, toRad(75), 0.1f, 100.0f);
    cam_move(win->camera, -3, 3, -3);
    cam_rotate(win->camera, toRad(-38.0f), toRad(-45.0f), 0);

    llog(INFO, "Starting render cycle");
    win_startRenderCycle(win);

    llog(INFO, "Shutting down application");
    win_dispose(win);
    return 0;
}
