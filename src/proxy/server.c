#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "server.h"
#include "request.h"
#include "response.h"
#include "proxy.h"

connection* create_http_connection(int conn_fd)
{
    connection* conn;

    conn = (connection *)malloc(sizeof(connection));
    if (conn == NULL)
        return NULL;
    conn->conn_fd = conn_fd;
    conn->buf_size = 0;
    conn->proxy_fd = -1;
    conn->proxy_buf_size = 0;
    conn->current_request = NULL;
    conn->queue = create_request_queue();
    if (conn->queue == NULL) {
        free(conn);
        return NULL;
    }
    conn->close = 0;
    conn->next = NULL;

    return conn;
}

void connection_free(connection* conn)
{
    close(conn->conn_fd);
    if (conn->proxy_fd != -1)
        close(conn->proxy_fd);
    if (conn->current_request != NULL)
        request_node_free(conn->current_request);
    if (conn->queue != NULL)
        request_queue_free(conn->queue);
    free(conn);
}

conn_list* create_connection_list()
{
    conn_list* list;

    list = (conn_list *)malloc(sizeof(conn_list));
    if (list == NULL)
        return NULL;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void connection_list_add(conn_list* list, connection* conn)
{
    if (list->head == NULL) {
        list->head = conn;
        list->tail = conn;
    } else {
        list->tail->next = conn;
        list->tail = conn;
    }
    list->size++;
}

void connection_list_remove(conn_list* list, int conn_fd)
{
    connection* conn;
    connection* temp;

    temp = NULL;
    conn = list->head;
    while (conn) {
        if (conn->conn_fd == conn_fd) {
            if (conn == list->head) {
                list->head = conn->next;
                if (conn == list->tail)
                    list->tail = NULL;
            } else if (conn == list->tail) {
                list->tail = temp;
                temp->next = NULL;
            } else {
                temp->next = conn->next;
            }
            connection_free(conn);
            list->size--;
            return;
        }
        temp = conn;
        conn = conn->next;
    }
}

void connection_list_free(conn_list* list)
{
    connection* conn;
    connection* temp;

    conn = list->head;
    while (conn) {
        temp = conn->next;
        connection_free(conn);
        conn = temp;
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
                 conn_list* list)
{
    int maxfd = 0;
    connection* conn;
    connection* temp;

    FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_SET(listen_sock, readfds);
    maxfd = listen_sock;
    conn = list->head;
    while (conn) {
        if (conn->close) {
            temp = conn->next;
            connection_list_remove(list, conn->conn_fd);
            conn = temp;
            continue;
        }
        FD_SET(conn->conn_fd, readfds);
        if (conn->queue->head != NULL && conn->queue->head->stage == READY)
            FD_SET(conn->conn_fd, writefds);
        if (conn->proxy_fd != -1) {
            FD_SET(conn->proxy_fd, readfds);
            if (conn->queue->head != NULL)
                FD_SET(conn->proxy_fd, writefds);
        }
        if (conn->conn_fd > maxfd)
            maxfd = conn->conn_fd;
        if (conn->proxy_fd > maxfd)
            maxfd = conn->proxy_fd;
        conn = conn->next;
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

void run_server(int port)
{
    int sock;
    fd_set readfds, writefds;
    int maxfd, readynum, n;
    conn_list* connection_list;
    connection* conn;
    connection* temp;

    sock = start_listen_sock(port);
    connection_list = create_connection_list();
    if (connection_list == NULL) {
        fprintf(stderr, "No memory to create connection list\n");
        exit(EXIT_FAILURE);
    }
    /* Begin the main loop waiting for things to do */
    while (1) {
        maxfd = update_fdset(&readfds, &writefds, sock, connection_list);
        if ((readynum =
                    select(maxfd + 1, &readfds, &writefds, NULL, NULL)) < 0) {
            fprintf(stderr, "Error on select\n");
            continue;
        }
        /* Accept new http connections */
        if (readynum > 0 && FD_ISSET(sock, &readfds)) {
            readynum--;
            accept_http_connection(sock, connection_list);
        }
        /* Traverse current connection list,
         *          * see if any connection needs to be deal with */
        conn = connection_list->head;
        while (conn && readynum > 0) {
            if (FD_ISSET(conn->conn_fd, &writefds)) {
                /* If this connection has something to send */
                readynum--;
                handle_connection_send(conn);
                /* If close after send, remove the connection */
                if (conn->close) {
                    fprintf(stderr, "Server closed the socket\n");
                    temp = conn->next;
                    connection_list_remove(connection_list, conn->conn_fd);
                    conn = temp;
                    continue;
                }
            }
            if (FD_ISSET(conn->conn_fd, &readfds)) {
                /* If this connection had something to recv */
                readynum--;
                if (conn->close) {
                    conn = conn->next;
                    continue;
                }
                n = http_recv(conn);
                if (n == -1) {
                    temp = conn->next;
                    connection_list_remove(connection_list, conn->conn_fd);
                    conn = temp;
                    continue;
                } else if (n == 0) {
                    conn = conn->next;
                    continue;
                } else {
                    conn->buf_size += n;
                    handle_connection_recv(conn);
                    handle_proxy_request(conn);
                }
            }
            conn = conn->next;
        }
    }
    close(sock);
    connection_list_free(connection_list);
}

void accept_http_connection(int sock, conn_list* connection_list)
{
    int client;
    struct sockaddr_in client_addr;
    socklen_t client_size;
    connection* conn;

    client_size = sizeof(client_addr);
    if ((client = accept(sock,
                         (struct sockaddr *)&client_addr,&client_size))==-1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            fprintf(stderr, "Error on accept\n");
        return;
    }
    if (!nonblock(client)) {
        fprintf(stderr, "Fail to set nonblock socket\n");
        close(client);
        return;
    }
    conn = create_http_connection(client);
    if (conn == NULL) {
        fprintf(stderr, "No memory to handle the new connection\n");
        close(client);
    } else {
        connection_list_add(connection_list, conn);
    }
}

int http_recv(connection* conn)
{
    int n;
    n = recv(conn->conn_fd, conn->buf + conn->buf_size,
             DEFAULT_BUF_SIZE - conn->buf_size, 0);
    if (n == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            fprintf(stderr, "Error on recv\n");
            return -1;
        }
        return 0;
    }
    if (n == 0) {
        return -1;
    }

    return n;
}

void handle_connection_recv(connection* conn)
{
    char line_buf[MAXLINE];

    while (1) {
        if (conn->current_request == NULL) {
            /* Start building a new request */
            conn->current_request = create_request_node();
            if (conn->current_request == NULL) {
                fprintf(stderr, "No memory for new request\n");
                conn->close = 1;
                return;
            }
        }
        if (conn->current_request->stage == ABORT) {
            /* Abort all the left message in the request */
            request_queue_add_node(conn->queue,
                                   conn->current_request);
            conn->current_request = NULL;
            return;
        }
        if (conn->current_request->stage == BODY) {
            /* Receive message body of request */
            read_message_body(conn);
            if (conn->current_request->stage == DONE) {
                /* Add request to queue */
                request_queue_add_node(conn->queue,
                                       conn->current_request);
                conn->current_request = NULL;
            }
            if(conn->buf_size == 0)
                break;
            continue;
        }
        if (connection_readline(conn, line_buf) == 0) {
            /* Can't read more lines from current buffer */
            if (conn->buf_size == DEFAULT_BUF_SIZE) {
                /* request head length > 8192 */
                conn->current_request->res->status_code = REQUEST_TOO_LARGE;
                conn->current_request->stage = ABORT;
                conn->current_request->close = 1;
                continue;
            }
            break;
        }
        if (conn->current_request->stage == START) {
            /* Parse the start line of the request */
            parse_request_line(conn->current_request, line_buf);
        } else if (conn->current_request->stage == HEADER) {
            /* Parse the headers of the request */
            parse_request_header(conn->current_request, line_buf);
            if (conn->current_request->stage == DONE) {
                /* Add request to queue */
                request_queue_add_node(conn->queue,
                                       conn->current_request);
                conn->current_request = NULL;
            }
        }
    }
}

void read_message_body(connection* conn)
{
    request_node* node = NULL;
    int bytes_need, i;

    node = conn->current_request;
    if (node->body_buf == NULL)
        node->body_buf = (char *)malloc(node->body_size);
    if (node->body_buf == NULL) {
        fprintf(stderr, "No memory to store request body");
        node->res->status_code = INTERNAL_SERVER_ERROR;
        node->stage = ABORT;
        node->close = 1;
        return;
    }
    bytes_need = node->body_size - node->body_read;
    if (bytes_need <= conn->buf_size) {
        memcpy(node->body_buf + node->body_read, conn->buf, bytes_need);
        node->body_read += bytes_need;
        for (i = bytes_need; i < conn->buf_size; i++)
            conn->buf[i-bytes_need] = conn->buf[i];
        conn->buf_size = conn->buf_size - bytes_need;
        node->stage = DONE;
    } else {
        memcpy(node->body_buf + node->body_read, conn->buf, conn->buf_size);
        node->body_read += conn->buf_size;
        conn->buf_size = 0;
    }
}

int connection_readline(connection* conn, char* line_buf)
{
    int i, j;

    for (i = 1; i < conn->buf_size; i++) {
        if (conn->buf[i-1] == '\r' && conn->buf[i] == '\n') {
            /* Read a line */
            memcpy(line_buf, conn->buf, i-1);
            line_buf[i-1] = '\0';
            /* Shift buffer to new data */
            for (j = i+1; j < conn->buf_size; j++)
                conn->buf[j-i-1] = conn->buf[j];
            conn->buf_size = conn->buf_size - i - 1;
            return 1;
        }
    }

    return 0;
}

void handle_connection_send(connection* conn)
{
    request_node* node = NULL;
    int bytes_sent;

    bytes_sent = 0;
    while (bytes_sent < DEFAULT_BUF_SIZE && conn->close == 0) {
        node = conn->queue->head;
        if (!node || node->stage != READY)
            break;
        /* Send response lines */
        if (send_response_lines(conn, node, &bytes_sent) == 0) {
            printf("send lines done\n");
            return;
        }
        /* Send the message body of the response */
        if (send_response_body(conn, node, &bytes_sent) == 0)
            return;
        /* Should close conn when finish sending node with close */
        if (node->close) {
            conn->close = 1;
            return;
        }
        node = request_queue_pop(conn->queue);
        request_node_free(node);
    }
}

int send_response_lines(connection *conn, request_node *node, int *bytes_sent)
{
    response *res = NULL;
    response_line* line = NULL;
    int line_len, testi, i;

    res = node->res;
    line = res->head;
    while (line) {
        line_len = strlen(line->data);
        testi = send(conn->conn_fd, line->data, line_len, 0);
        if (testi == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                fprintf(stderr, "Error on send\n");
                conn->close = 1;
                return 0;
            }
            fprintf(stderr, "Send buffer full before sending\n");
            return 0;
        }
        if (testi < line_len) {
            fprintf(stderr, "Send buffer full while sending\n");
            for (i = testi; i <= line_len; i++)
                line->data[i-testi] = line->data[i];
            return 0;
        }
        *bytes_sent += line_len;
        line = response_pop_line(res);
        free(line);
        line = res->head;
    }
    if (*bytes_sent >= DEFAULT_BUF_SIZE) {
        if (res->head == NULL && node->body_buf == NULL) {
            if (node->close)
                conn->close = 1;
            node = request_queue_pop(conn->queue);
            request_node_free(node);
        }
        return 0;
    }

    return 1;
}

int send_response_body(connection *conn, request_node *node, int *bytes_sent)
{
    int testi, n;
    response *res = NULL;

    res = node->res;
    if (res->body_buf == NULL || res->body_sent == res->body_size)
        return 1;
    n = res->body_size - res->body_sent;
    if (n > DEFAULT_BUF_SIZE - *bytes_sent)
        n = DEFAULT_BUF_SIZE - *bytes_sent;
    testi = send(conn->conn_fd, res->body_buf + res->body_sent, n, 0);
    if (testi == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            fprintf(stderr, "Error on send\n");
            conn->close = 1;
            return 0;
        }
        fprintf(stderr, "Send buffer full before sending\n");
        return 0;
    }
    if (testi < n) {
        fprintf(stderr, "Send buffer full while sending");
        res->body_sent += testi;
        return 0;
    }
    *bytes_sent += testi;
    res->body_sent = res->body_size;
    return 1;
}
