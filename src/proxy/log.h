#ifndef LOG_H
#define LOG_H

/**
 * Initialize the log file to a given path
 */
void log_initialize(char* filename);

/*
 * Log a formated string to the log file
 */
void log_log(char* str, ...);

/**
 * Close the log file
 */
void log_close();

#endif
