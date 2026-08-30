#ifndef COMMON_H
#define COMMON_H

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h> // For printf
#include <stdarg.h>

    void setupLogs(void);
    void printErrno(const char *prefix, const char *format, ...);

    void _printLog(FILE *f, const char *prefix, const char *format, ...);

    size_t getByToken(const char *line, size_t lineLength, int offset, char token);
    void printNum(const char *str, int num);
#if __cplusplus
}
#endif

#define printError(...) _printLog(stderr, __VA_ARGS__)
#define printLog(...) _printLog(stdout, __VA_ARGS__)

#endif