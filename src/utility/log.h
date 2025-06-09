#ifndef LOG_H
#define LOG_H
#include <stdio.h>

#define RESET_C   "\033[0m"
#define GREEN_C   "\033[32m"
#define RED_C     "\033[31m"
#define WHITE_C   "\033[37m"

extern const char *const INFO;
extern const char *const ERROR;

void putLogMessage(const char *level, const char *format, const char *module, va_list args);

#endif //LOG_H
