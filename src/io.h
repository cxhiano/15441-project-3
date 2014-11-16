/** @file io.h
 *  @brief Header file for io.c
 *
 *  @author Chao Xin(cxin)
 */
#ifndef __MYIO_H__
#define __MYIO_H__

#include <unistd.h>

/*
 * Initial buffer size
 */
#define BUFSIZE 1024

/** @brief Context for using select */
typedef struct {
    fd_set read_fds, read_fds_cpy;
    fd_set write_fds, write_fds_cpy;
    int fd_max;
} select_context;

/* Select context */
int io_select();       // Shorthand for select
void init_select_context();
void add_read_fd(int fd);
void remove_read_fd(int fd);
int test_read_fd(int fd);
void add_write_fd(int fd);
void remove_write_fd(int fd);
int test_write_fd(int fd);


int io_readline(int fd, char* buf, int bufsize);

#endif
