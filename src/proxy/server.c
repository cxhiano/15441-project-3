#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "transaction.h"
#include "proxy.h"

proxy_session *create_session(int client_fd)
{
    proxy_session *session;

    session = (proxy_session *)malloc(sizeof(proxy_session));
    if (session == NULL)
        return NULL;
    session->client_conn = create_connection(client_fd);
    if (session->client_conn == NULL) {
        free(session);
        return NULL;
    }
    session->server_conn = create_connection(-1);
    if (session->server_conn == NULL) {
        connection_free(session->client_conn);
        free(session);
        return NULL;
    }
    session->queue = create_transaction_queue();
    if (session->queue == NULL) {
        connection_free(session->client_conn);
        connection_free(session->server_conn);
        free(session);
        return NULL;
    }
    session->video = NULL;
    session->next = NULL;
    session->close = 0;

    return session;
}

void session_free(proxy_session *session)
{
    if (session->client_conn != NULL) {
        connection_free(session->client_conn);
    }
    if (session->server_conn != NULL) {
        connection_free(session->server_conn);
    }
    if (session->queue != NULL) {
        transaction_queue_free(session->queue);
    }
    if (session->video != NULL) {
        free(session->video);
    }
    free(session);
}

session_list* create_session_list()
{
    session_list* list;

    list = (session_list *)malloc(sizeof(session_list));
    if (list == NULL)
        return NULL;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void session_list_add(session_list* list, proxy_session* session)
{
    if (list->head == NULL) {
        list->head = session;
        list->tail = session;
    } else {
        list->tail->next = session;
        list->tail = session;
    }
}

void session_list_remove(session_list *list, proxy_session *session)
{
    proxy_session* sess;
    proxy_session* temp;

    temp = NULL;
    sess = list->head;
    while (sess) {
        if (sess == session) {
            if (sess == list->head) {
                list->head = sess->next;
                if (sess == list->tail) {
                    list->tail = NULL;
                }
            } else if (sess == list->tail) {
                list->tail = temp;
                temp->next = NULL;
            } else {
                temp->next = sess->next;
            }
            session_free(sess);
            return;
        }
        temp = sess;
        sess = sess->next;
    }
}

void session_list_free(session_list* list)
{
    proxy_session* sess;
    proxy_session* temp;

    sess = list->head;
    while (sess) {
        temp = sess->next;
        session_free(sess);
        sess = temp;
    }
    free(list);
}

int nonblock(int sock)
{
    int flags;
    if ((flags = fcntl(sock, F_GETFL)) < 0) {
        fprintf(stderr, "Fail to get flags by fcntl");
        return 0;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "Fail to set flags by fcntl");
        return 0;
    }
    return 1;
}

int update_fdset(fd_set* readfds, fd_set* writefds, int listen_sock,
                 session_list* list)
{
    int maxfd = 0;
    proxy_session* session;
    proxy_session* temp;

    FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_SET(listen_sock, readfds);
    maxfd = listen_sock;
    session = list->head;
    while (session) {
        if (session->close) {
            temp = session->next;
            session_list_remove(list, session);
            session = temp;
            continue;
        }
        FD_SET(session->client_conn->conn_fd, readfds);
        if (session->queue->head != NULL &&
                session->queue->head->stage == READY)
            FD_SET(session->client_conn->conn_fd, writefds);
        if (session->server_conn->conn_fd != -1) {
            FD_SET(session->server_conn->conn_fd, readfds);
            if (session->queue->head != NULL)
                FD_SET(session->server_conn->conn_fd, writefds);
        }
        if (session->client_conn->conn_fd > maxfd)
            maxfd = session->client_conn->conn_fd;
        if (session->server_conn->conn_fd > maxfd)
            maxfd = session->server_conn->conn_fd;
        session = session->next;
    }
    return maxfd;
}

int start_listen_sock(int port)
{
    int sock;
    struct sockaddr_in addr;

    /* Start listen socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Fail to create socket\n");
        exit(EXIT_FAILURE);
    }
    if (!nonblock(sock)) {
        fprintf(stderr, "Fail to set nonblock socket\n");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))) {
        fprintf(stderr, "Fail to bind socket\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (listen(sock, LISTEN_BACKLOG)) {
        fprintf(stderr, "Fail to listen on socket");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

void accept_http_connection(int sock, session_list* session_list)
{
    int client;
    struct sockaddr_in client_addr;
    socklen_t client_size;
    proxy_session* session;

    client_size = sizeof(client_addr);
    if ((client = accept(sock, (struct sockaddr *)&client_addr,
                         &client_size)) == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            fprintf(stderr, "Error on accept\n");
        return;
    }
    if (!nonblock(client)) {
        fprintf(stderr, "Fail to set nonblock socket\n");
        close(client);
        return;
    }
    session = create_session(client);
    if (session == NULL) {
        fprintf(stderr, "No memory to handle the new connection\n");
        close(client);
    } else {
        session_list_add(session_list, session);
    }
}

void run_server(int port)
{
    int sock;
    fd_set readfds, writefds;
    int maxfd, readynum;
    session_list* session_list;
    proxy_session* session;
    proxy_session* temp;

    sock = start_listen_sock(port);
    session_list = create_session_list();
    if (session_list == NULL) {
        fprintf(stderr, "No memory to create session list\n");
        exit(EXIT_FAILURE);
    }
    /* Begin the main loop waiting for things to do */
    while (1) {
        maxfd = update_fdset(&readfds, &writefds, sock, session_list);
        if ((readynum =
                    select(maxfd + 1, &readfds, &writefds, NULL, NULL)) < 0) {
            fprintf(stderr, "Error on select\n");
            continue;
        }
        /* Accept new http connections */
        if (readynum > 0 && FD_ISSET(sock, &readfds)) {
            readynum--;
            accept_http_connection(sock, session_list);
        }
        /* Traverse current connection list,
         *          * see if any connection needs to be deal with */
        session = session_list->head;
        while (session && readynum > 0) {
            if (FD_ISSET(session->client_conn->conn_fd, &writefds)) {
                /* If this connection has something to send */
                readynum--;
                //TODO send responses
                handle_client_send(session);
                /* If close after send, remove the connection */
                if (session->close) {
                    temp = session->next;
                    session_list_remove(session_list, session);
                    session = temp;
                    continue;
                }
            }
            if (FD_ISSET(session->server_conn->conn_fd, &writefds)) {
                readynum--;
                if (session->close) {
                    session = session->next;
                    continue;
                }
                handle_server_send(session);
                if (session->close) {
                    temp = session->next;
                    session_list_remove(session_list, session);
                    session = temp;
                    continue;
                }
            }
            if (FD_ISSET(session->client_conn->conn_fd, &readfds)) {
                /* If this connection had something to recv */
                readynum--;
                if (session->close) {
                    session = session->next;
                    continue;
                }
                handle_client_recv(session);
                if (session->close) {
                    temp = session->next;
                    session_list_remove(session_list, session);
                    session = temp;
                    continue;
                }
            }
            if (FD_ISSET(session->server_conn->conn_fd, &readfds)) {
                readynum--;
                if (session->close) {
                    session = session->next;
                    continue;
                }
                handle_server_recv(session);
                if (session->close) {
                    temp = session->next;
                    session_list_remove(session_list, session);
                    session = temp;
                    continue;
                }
            }
            session = session->next;
        }
    }
    close(sock);
    session_list_free(session_list);
}

void handle_client_recv(proxy_session *session)
{
    int n;
    message *msg = NULL;
    transaction_node *node = NULL;

    n = connection_recv(session->client_conn);
    if (n == -1) {
        session->close = 1;
        return;
    } else if (n == 0) {
        return;
    } else {
        while ((msg = connection_read_msg(session->client_conn)) != NULL) {
            if ((node = create_transaction_from_msg(msg)) == NULL) {
                message_free(msg);
                session->close = 1;
                return;
            }
            if (node->status_code != 0) {
                generate_error_response(node);
                if (node->response_msg == NULL) {
                    transaction_node_free(node);
                    session->close = 1;
                    return;
                }
                node->stage = READY;
                transaction_queue_add_node(session->queue, node);
                break;
            }
            transaction_queue_add_node(session->queue, node);
        }
        handle_proxy_session(session);
    }
}

void handle_server_send(proxy_session *session)
{
    transaction_node *node;
    int testi;

    node = session->queue->head;
    while (node) {
        if (node->stage == PROXY) {
            testi = connection_send_msg(session->server_conn,node->request_msg);
            if (testi == 0) {
                node->stage = DONE;
            } else if (testi == -1) {
                break;
            } else if (testi == -2) {
                session->close = 1;
                return;
            }
        }
        if (node->close)
            break;
        node = node->next;
    }
}

void handle_server_recv(proxy_session *session)
{
    int n;
    message *msg = NULL;
    transaction_node *node = NULL;

    n = connection_recv(session->server_conn);
    if (n == -1) {
	session->close = 1;
        return;
    } else if (n == 0) {
        return;
    } else {
        while ((msg = connection_read_msg(session->server_conn)) != NULL) {
            node = session->queue->head;
            while (node) {
                if (node->stage == DONE) {
                    node->finish_time = now();
                    node->response_msg = msg;
                    node->stage= READY;
                    break;
                }
                node = node->next;
            }
            if (node == NULL) {
                fprintf(stderr, "Discard server response message\n");
                message_free(msg);
            }
        }
    }
}

void handle_client_send(proxy_session *session)
{
    transaction_node *node;
    message_line *cookie_line;
    int testi;
    char cookie_buf[MAXLINE];

    node = session->queue->head;
    while (node && node->stage == READY) {
        if (node->special) {
            //TODO parse f4m
            session->video = create_video();
            if (session->video != NULL) {
                parse_bitrates(session, node);
            }
        } else {
            if (strcmp(node->extension, "f4m") == 0) {
                sprintf(cookie_buf, "Set-Cookie: filename=%s\r\n",
                        node->filename);
                cookie_line = create_message_line(cookie_buf);
                if (cookie_line != NULL) {
                    cookie_line->next = node->response_msg->head->next;
                    node->response_msg->head->next = cookie_line;
                }
            }
            testi = connection_send_msg(session->client_conn,node->response_msg);
            if (testi == 0) {
                //TODO update log and throughput
                if (node->start_time != 0) {
                    update_throughput(session, node);
                }
            } else if (testi == -1) {
                break;
            } else if (testi == -2) {
                session->close = 1;
                return;
            }
        }
        if (node->close) {
            session->close = 1;
            return;
        }
        node = transaction_queue_pop(session->queue);
        transaction_node_free(node);
        node = session->queue->head;
    }
    handle_proxy_session(session);
}
