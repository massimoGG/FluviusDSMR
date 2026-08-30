/**
 * common.c - Commonly used functions that can be used anywhere
 */
#include <stdio.h>  // For printf
#include <stdlib.h> // For exit
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#include "common.h"

/**
 * setupLogs sets up the locale of the user terminal
 */
void setupLogs(void)
{
    setlocale(LC_ALL, "");
}

/**
 * printErrno prints the given prefix and format followed by
 * the error number and errno string meaning
 */
void printErrno(const char *prefix, const char *format, ...)
{
    va_list args;

    fprintf(stderr, "%s:\t", prefix);

    // Fetch errno value
    const int errsv = errno;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n\terrno %d: %s\n\n", errsv, strerror(errsv));
}
/**
 * printLog prints the given prefix and format using variable arguments
 */
void _printLog(FILE *f, const char *prefix, const char *format, ...)
{
    va_list va;
    fprintf(f, "%s:\t", prefix);

    va_start(va, format);
    vfprintf(f, format, va);
    va_end(va);

    fprintf(f, "\n");
}

size_t getByToken(const char *line, size_t lineLength, int offset, char token)
{
    for (size_t i = offset; i < lineLength; i++)
    {
        if (line[i] == token)
            return i;
    }
    return lineLength;
}

void printNum(const char *str, int num)
{
    for (int i = 0; i < num; i++)
        printf("%c", str[i]);
    printf("\n");
}