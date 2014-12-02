#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "io.h"

connection *create_connection(int conn_fd)
{
    connection* conn;

    conn = (connection *)malloc(sizeof(connection));
    if (conn == NULL)
        return NULL;
    conn->conn_fd = conn_fd;
    conn->buf_size = 0;
    conn->current_msg = NULL;

    return conn;
}

void connection_free(connection* conn)
{
    if (conn->conn_fd != -1) {
        close(conn->conn_fd);
    }
    if (conn->current_msg != NULL) {
        message_free(conn->current_msg);
    }
    free(conn);
}

int connection_recv(connection *conn)
{
    int n;
    n = recv(conn->conn_fd, conn->buf + conn->buf_size,
             MAXLINE - conn->buf_size, 0);
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
    conn->buf_size += n;

    return n;
}

int connection_readline(connection* conn, char* line_buf)
{
    int i, j;

    for (i = 1; i < conn->buf_size; i++) {
        if (conn->buf[i-1] == '\r' && conn->buf[i] == '\n') {
            /* Read a line */
            memcpy(line_buf, conn->buf, i+1);
            line_buf[i+1] = '\0';
            /* Shift buffer to new data */
            for (j = i+1; j < conn->buf_size; j++)
                conn->buf[j-i-1] = conn->buf[j];
            conn->buf_size = conn->buf_size - i - 1;
            return 1;
        }
    }

    return 0;
}

message *connection_read_msg(connection *conn)
{
    message_line *line = NULL;
    message *result = NULL;
    message *msg = NULL;
    char line_buf[MAXLINE];
    int bytes_need, i;

    while (1) {
        if (conn->current_msg == NULL) {
            if ((conn->current_msg = create_http_message()) == NULL) {
                fprintf(stderr, "No memory for new request\n");
                return NULL;
            }
        }
        if (conn->current_msg->stage == MSG_ABORT) {
            result = conn->current_msg;
            conn->current_msg = NULL;
            return result;
        }
        if (conn->current_msg->stage == MSG_BODY) {
            if (conn->current_msg->body_buf == NULL) {
                if ((conn->current_msg->body_buf =
                            (char *)malloc(conn->current_msg->body_size)) == NULL) {
                    conn->current_msg->stage = MSG_ABORT;
                    continue;
                }
            }
            msg = conn->current_msg;
            bytes_need = msg->body_size - msg->body_read;
            if (bytes_need <= conn->buf_size) {
                memcpy(msg->body_buf + msg->body_read, conn->buf, bytes_need);
                msg->body_read += bytes_need;
                for (i = bytes_need; i < conn->buf_size; i++)
                    conn->buf[i-bytes_need] = conn->buf[i];
                conn->buf_size = conn->buf_size - bytes_need;
                msg->stage = MSG_DONE;
            } else {
                memcpy(msg->body_buf + msg->body_read, conn->buf,
                       conn->buf_size);
                msg->body_read += conn->buf_size;
                conn->buf_size = 0;
            }
            if (conn->current_msg->stage == MSG_DONE) {
                result = conn->current_msg;
                conn->current_msg = NULL;
                return result;
            }
            if (conn->buf_size == 0) {
                break;
            }
            continue;
        }
        if (connection_readline(conn, line_buf) == 0) {
            /* Can't read more lines from current buffer */
            if (conn->buf_size == MAXLINE) {
                /* message line length > MAXLINE */
                conn->current_msg->stage = MSG_ABORT;
                continue;
            }
            break;
        }
        if (conn->current_msg->stage == MSG_LINE) {
            if ((line = create_message_line(line_buf)) == NULL) {
                conn->current_msg->stage = MSG_ABORT;
                continue;
            }
            message_add_line(conn->current_msg, line);
            if (line_buf[0] == '\r' && line_buf[1] == '\n') {
                /* Message lines have been fully received */
                if (conn->current_msg->body_size > 0) {
                    conn->current_msg->stage = MSG_BODY;
                    continue;
                } else {
                    conn->current_msg->stage = MSG_DONE;
                    result = conn->current_msg;
                    conn->current_msg = NULL;
                    return result;
                }
            }
            if (strstr(line_buf, "Content-Length:") == line_buf) {
                sscanf(line_buf, "Content-Length: %d",
                       &(conn->current_msg->body_size));
                if (conn->current_msg->body_size > MAX_BODY_SIZE) {
                    conn->current_msg->stage = MSG_ABORT;
                    continue;
                }
                if (conn->current_msg->body_size < 0) {
                    conn->current_msg->stage = MSG_ABORT;
                    continue;
                }
            }
        }
    }
    return result;
}

int connection_send_msg(connection *conn, message *msg)
{
    message_line *line = NULL;
    int line_len, testi, i;

    line = msg->head;
    while (line) {
        line_len = strlen(line->data);
        testi = send(conn->conn_fd, line->data, line_len, 0);
        if (testi == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                fprintf(stderr, "Error on send\n");
                return -2;
            }
            fprintf(stderr, "Send buffer full before sending\n");
            return -1;
        }
        if (testi < line_len) {
            fprintf(stderr, "Send buffer full while sending\n");
            for (i = testi; i <= line_len; i++)
                line->data[i-testi] = line->data[i];
            return -1;
        }
        line = message_pop_line(msg);
        free(line);
        line = msg->head;
    }
    if (msg->body_size > msg->body_sent && msg->body_buf != NULL) {
        testi = send(conn->conn_fd, msg->body_buf + msg->body_sent,
                     msg->body_size - msg->body_sent, 0);
        if (testi == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                fprintf(stderr, "Error on send\n");
                return -2;
            }
            fprintf(stderr, "Send buffer full before sending\n");
            return -1;
        }
        if (testi < msg->body_size - msg->body_sent) {
            fprintf(stderr, "Send buffer full while sending");
            msg->body_sent += testi;
            return -1;
        }
        msg->body_sent = msg->body_size;
    }
    return 0;
}
