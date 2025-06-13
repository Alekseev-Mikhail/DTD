#include "log.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

const char *const INFO = GREEN_C "INFO" RESET_C;
const char *const ERROR = RED_C "ERROR" RESET_C;

void glog(const char *const level, const char *const format, const char *const module, va_list args) {
    time_t rawTime;
    time(&rawTime);
    const struct tm *timeInfo = localtime(&rawTime);

    printf(
        "%d-%d-%d %d:%d:%d %s " WHITE_C "(%s)" RESET_C ": ",
        1900 + timeInfo->tm_year,
        timeInfo->tm_mon,
        timeInfo->tm_mday,
        timeInfo->tm_hour,
        timeInfo->tm_min,
        timeInfo->tm_sec,
        level,
        module
    );

    vprintf(format, args);
    printf("\n");
}
