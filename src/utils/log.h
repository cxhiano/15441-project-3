/** @file log.h
 *  @brief Defines functions for logging
 *
 *  @author Chao Xin(cxin)
 */
#ifndef __MYLOG_H__
#define __MYLOG_H__

#include <stdio.h>

#define L_ERROR 0x1 //This flag indicates an error message
#define L_INFO 0x2 //This flag indicates an info message
#define L_DEBUG 0x4 //This flag indicates an info message
/* User define debug flag */

/*
 * @brief log_file indicates where to write log. When it's NULL, log will be written
 *        to stderr
 */
FILE* log_file;
/*
 * @brief A bit map of what types of logs should be output
 */
int log_mask;

/** @brief Set the file where logs will be output to
 *
 *  @param fname The log file name
 *  @return void
 */
void set_log_file(char* fname);

/** @brief Write formatted logs
 *
 *  Create a new log message with given type(Defined in log.h). The message
 *  will be written into log file if the type is in log_mask.
 *
 *  The output in this function is implemented using vfprintf. Thus this
 *  function can be used like printf(char* format, ... )
 *
 *  When log_file is NULL, the message will be output to stderr
 *
 *  @param type The type of the log message
 *  @param format The output format
 *  @oaram ... Arguments for the format
 *
 *  @return void
 */
void log_msg(int type, char* format, ...);


/** @brief Write an error log
 *
 *  Similar to perror(). The string explanation of errno will be appended to the
 *  passed-in message.
 *
 *  @param msg - The message about the error
 *
 *  @return void
 */
void log_error(char* msg);

#endif
