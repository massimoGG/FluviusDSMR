#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

    void setupLogs(void);
    void printErrno(const char *prefix, const char *format, ...);

    void _printLog(FILE *f, const char *prefix, const char *format, ...);

#define printError(...) _printLog(stderr, __VA_ARGS__)
#define printLog(...) _printLog(stdout, __VA_ARGS__)

    int getByToken(char *line, int lineLength, int offset, char token);
    void printNum(char *str, int num);

#ifdef __cplusplus
}
#endif

#endif