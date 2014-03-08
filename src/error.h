#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

void error_set_log_file(FILE *file);
void log(const char *format, ...);
void warning(const char *format, ...);
int error(const char *format, ...);
int error_errno(const char *format, ...);
void fatal(const char *format, ...);
void fatal_errno(const char *format, ...);

#endif
