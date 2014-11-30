#ifndef SERVER_H
#define SERVER_H

#include "request.h"

#include <sys/select.h>

#define LISTEN_BACKLOG 128
#define DEFAULT_BUF_SIZE 8192
#define TIME_OUT 300

typedef struct connection {
    int conn_fd;
    int buf_size;
    char buf[DEFAULT_BUF_SIZE];
    int proxy_fd;
    int proxy_buf_size;
    char proxy_buf[DEFAULT_BUF_SIZE];
    struct request_node* current_request;
    struct request_queue* queue;
    struct connection* next;
    char close;
} connection;

typedef struct conn_list {
    int size;
    connection* head;
    connection* tail;
} conn_list;

/**
 * Create a connection structure for the given socket fd
 */
connection* create_http_connection(int conn_fd);
/**
 * Finalize the connection
 */
void connection_free(connection* conn);
/**
 * Create a connection list
 */
conn_list* create_connection_list();
/**
 * Add a connection to a connection list
 */
void connection_list_add(conn_list* list, connection* conn);
/**
 * Remove a connection from the connection list according to its socket fd
 */
void connection_list_remove(conn_list* list, int conn_fd);
/**
 * Finalize a whole connection list and all its connections
 */
void connection_list_free(conn_list* list);

/**
 * Set a socket to nonblock mode
 */
int nonblock(int sock);
/**
 * Update the select fd set according to current connection list
 */
int update_fdset(fd_set* readfds, fd_set* writefds, int listen_sock,
        conn_list* list);
/**
 * Start a server listen socket at the given port
 */
int start_listen_sock(int port);
/**
 * Run the proxy server at the given port
 */
void run_server(int port);
/**
 * Accept a new http connection from the given socket.
 * Then add it to the current connection list.
 */
void accept_http_connection(int sock, conn_list* connection_list);
/**
 * Read the buffered data from the socket in the given connection
 */
int http_recv(connection* conn);

/**
 * Handle the current received data of the connection
 */
void handle_connection_recv(connection* conn);
/**
 * Read the message body of a request
 */
void read_message_body(connection* conn);
/**
 * Read a line ended with \r\n from the connection buffer
 */
int connection_readline(connection* conn, char* line_buf);
/**
 * Send all available responses of a connection
 */
void handle_connection_send(connection* conn);
/**
 * Send the response lines of the response of a request
 */
int send_response_lines(connection *conn, request_node *node, int *bytes_sent);
/**
 * Send the response body of the response of a request
 */
int send_response_body(connection *conn, request_node *node, int *bytes_sent);
#endif
