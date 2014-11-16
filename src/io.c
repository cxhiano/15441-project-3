/** @file io.c
 *  @brief Provides functions to read data from socket or write data to socket
 *
 *  The function take a greedy approach, that is, send and receive as much bytes
 *  as possible in one call. When receiving, the buffer size will increase
 *  dynamically. When sending, the buffer size might shrink when it's empty
 *  enough.
 *
 *  @author Chao Xin(cxin)
 */
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "io.h"
#include "log.h"

static select_context context;

void init_select_context() {
    FD_ZERO(&context.read_fds);
    FD_ZERO(&context.read_fds_cpy);
    FD_ZERO(&context.write_fds);
    FD_ZERO(&context.write_fds_cpy);
}

void add_read_fd(int fd) {
    FD_SET(fd, &context.read_fds_cpy);
    if (fd > context.fd_max)
        context.fd_max = fd;
}

void remove_read_fd(int fd) {
    FD_CLR(fd, &context.read_fds_cpy);
}

int test_read_fd(int fd) {
    return FD_ISSET(fd, &context.read_fds);
}

void add_write_fd(int fd) {
    FD_SET(fd, &context.write_fds_cpy);
    if (fd > context.fd_max)
        context.fd_max = fd;
}

void remove_write_fd(int fd) {
    FD_CLR(fd, &context.write_fds_cpy);
}

int test_write_fd(int fd) {
    return FD_ISSET(fd, &context.write_fds);
}

/** @brief Wrapper for select()
 *
 *  @return What select() returns
 */
int io_select() {
    context.read_fds = context.read_fds_cpy;
    context.write_fds = context.write_fds_cpy;
    return select(context.fd_max + 1, &context.read_fds, &context.write_fds,
        NULL, NULL);
}

int io_readline(int fd, char* buf, int bufsize) {
    int i, ret;
    char c;

    for (i = 0; i < bufsize; ++i) {
        ret = read(fd, &c, 1);
        if (ret == 1) {
            buf[i] = c;
            if (c == '\n')
                return i + 1;
        } else if (ret == 0) {
            return i;
        } else if (ret == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                log_error("io_readline: read error");
                return -1;
            }
            return i;
        }
    }

    return bufsize;
}
