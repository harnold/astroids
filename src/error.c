#include "error.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static FILE *error_log_file;

static void vprinterr(const char *prefix, int errnum, const char *format,
                      va_list args)
{
    static char msg[1024];

    vsnprintf(msg, sizeof(msg), format, args);
    fflush(stdout);

    FILE *log_file;

    if (error_log_file != NULL)
        log_file = error_log_file;
    else
        log_file = stderr;

    if (errnum != 0)
        fprintf(log_file, "%s%s: %s\n", prefix, msg, strerror(errnum));
    else
        fprintf(log_file, "%s%s\n", prefix, msg);
}

void error_set_log_file(FILE *file)
{
    error_log_file = file;
}

void log(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprinterr("Log: ", 0, format, args);
    va_end(args);
}

void warning(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprinterr("Warning: ", 0, format, args);
    va_end(args);
}

int error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprinterr("Error: ", 0, format, args);
    va_end(args);

    return -1;
}

int error_errno(const char *format, ...)
{
    int errnum = errno;
    va_list args;

    va_start(args, format);
    vprinterr("Error: ", errnum, format, args);
    va_end(args);

    return -1;
}

void fatal(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprinterr("Fatal error: ", 0, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

void fatal_errno(const char *format, ...)
{
    int errnum = errno;
    va_list args;

    va_start(args, format);
    vprinterr("Fatal error: ", errnum, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}
