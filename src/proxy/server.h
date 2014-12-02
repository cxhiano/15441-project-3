#ifndef SERVER_H
#define SERVER_H

#include "io.h"
#include <sys/select.h>

#define LISTEN_BACKLOG 128

typedef struct proxy_session {
    struct video_bitrates *video;
    struct connection *client_conn;
    struct connection *server_conn;
    struct transaction_queue* queue;
    struct proxy_session *next;
    char close;
} proxy_session;

typedef struct session_list {
    proxy_session *head;
    proxy_session *tail;
} session_list;

/**
 * Create a session structure for the given client fd
 */
proxy_session *create_session(int client_fd);
/**
 * Finalize the session and close its connection sockets
 */
void session_free(proxy_session *session);
/**
 * Create a session list
 */
session_list* create_session_list();
/**
 * Add a session to a session list
 */
void session_list_add(session_list *list, proxy_session *session);
/**
 * Remove a session from the session list
 */
void session_list_remove(session_list *list, proxy_session *session);
/**
 * Finalize a whole session list and all its sessions
 */
void session_list_free(session_list *list);

/**
 * Set a socket to nonblock mode
 */
int nonblock(int sock);
/**
 * Update the select fd set according to current connection list
 */
int update_fdset(fd_set* readfds, fd_set* writefds, int listen_sock,
                 session_list* list);
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
 * Then add it to the current session list.
 */
void accept_http_connection(int sock, session_list* list);
/**
 * Send current available responses to client
 */
void handle_client_send(proxy_session *session);
/**
 * Forward current available requests to server
 */
void handle_server_send(proxy_session *session);
/**
 * Read current available requests from client socket
 */
void handle_client_recv(proxy_session *session);
/**
 * Read current available responses from server socket
 */
void handle_server_recv(proxy_session *session);

#endif
