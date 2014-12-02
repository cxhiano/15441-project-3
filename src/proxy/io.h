#ifndef IO_H
#define IO_H

#include "message.h"

typedef struct connection {
    int conn_fd;
    int buf_size;
    char buf[MAXLINE];
    struct message *current_msg;
} connection;

/**
 * Create a connection structure for the given socket fd
 */
connection *create_connection(int conn_fd);
/**
 * Finalize the connection
 */
void connection_free(connection *conn);
/**
 * Read raw data from the connection socket
 * Return the number of bytes that's read, -1 on error
 */
int connection_recv(connection *conn);
/**
 * Read a line ended with \r\n form connection buffer
 */
int connection_readline(connection *conn, char *line_buf);
/**
 * Try to read a message from the connection
 * Return the message on success, or NULL if more data need to be received
 * for a complete message. (Incomplete message is temporarily stored in the
 * connection structure) The caller is responsible to free the message.
 */
message *connection_read_msg(connection *conn);
/**
 * Try to send a message to the connection
 * Return 0 on success, or -1 if the message was not completely sent (due to
 * the network buffer limit). The part that has been sent will be removed
 * from the message.
 * Return -2 for severe error and the session should be terminated.
 */
int connection_send_msg(connection *conn, message *msg);

#endif
