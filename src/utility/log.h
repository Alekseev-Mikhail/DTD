#ifndef LOG_H
#define LOG_H
#include <stdio.h>

#define RESET_C   "\033[0m"
#define GREEN_C   "\033[32m"
#define YELLOW_C  "\033[33m"
#define BLUE_C    "\033[36m"
#define RED_C     "\033[31m"
#define PURPLE_C  "\033[35m"
#define WHITE_C   "\033[37m"

extern const char *const INFO;
extern const char *const DEBUG;
extern const char *const WARN;
extern const char *const ERROR;
extern const char *const FATAL;

void glog(const char *level, const char *format, const char *module, va_list args);

#endif //LOG_H
