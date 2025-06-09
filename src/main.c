#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "window.h"
#include "utility/log.h"

void render();

__attribute__ ((format(printf, 2, 3)))
static void llog(const char *const level, char *const format, ...) {
    va_list args;
    va_start(args, format);
    putLogMessage(level, format, "main", args);
    va_end(args);
}

int main(const int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Directory for resources was not defined. Try pass a path as a program argument.");
        abort();
    }
    resourceDirectory = argv[1];

    llog(INFO, "Initializing window");
    WindowData *data = malloc(sizeof(WindowData));
    win_initWindow(data, 500, 500, "Hiya, OpenGL!");

    llog(INFO, "Starting render cycle");
    win_startupRenderCycle(data);

    llog(INFO, "Shutting down application");
    win_dispose(data);
    return 0;
}
