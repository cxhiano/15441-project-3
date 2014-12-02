#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

FILE* log_fp = NULL;

void log_initialize(char* filename)
{
    log_fp = fopen(filename, "w");
    if (log_fp == NULL) {
        fprintf(stdout, "Invalid log file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void log_log(char* str, ...)
{
    va_list a_list;

    if (log_fp == NULL) {
        fprintf(stderr, "Log fail: Null pointer to log file\n");
        return;
    }

    va_start(a_list, str);
    vfprintf(log_fp, str, a_list);
    va_end(a_list);
    fflush(log_fp);
    return;
}

void log_close()
{
    fclose(log_fp);
}
