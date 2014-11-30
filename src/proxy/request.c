#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"

request_node* create_request_node()
{
    request_node* node;

    node = (request_node *)malloc(sizeof(request_node));
    if (node == NULL)
        return NULL;
    node->res = create_response();
    if (node->res == NULL) {
        free(node);
        return NULL;
    }
    node->method = NONE;
    node->stage = START;
    node->uri[0] = '\0';
    node->query[0] = '\0';
    node->headers.accept[0] = '\0';
    node->headers.accept_charset[0] = '\0';
    node->headers.accept_encoding[0] = '\0';
    node->headers.accept_language[0] = '\0';
    node->headers.content_type[0] = '\0';
    node->headers.cookie[0] = '\0';
    node->headers.host[0] = '\0';
    node->headers.referer[0] = '\0';
    node->headers.user_agent[0] = '\0';
    node->close = 0;
    node->header_num = 0;
    node->body_size = -1;
    node->body_read = 0;
    node->body_write = 0;
    node->body_buf = NULL;
    node->next = NULL;

    return node;
}

void request_node_free(request_node* node)
{
    if (node->body_buf != NULL)
        free(node->body_buf);
    if (node->res != NULL)
        response_free(node->res);
    free(node);
}

request_queue* create_request_queue()
{
    request_queue* queue;

    queue = (request_queue *)malloc(sizeof(request_queue));
    if (queue == NULL)
        return NULL;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void request_queue_add_node(request_queue* queue, request_node* node)
{
    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

request_node* request_queue_pop(request_queue* queue)
{
    request_node* node;

    if (queue->head == NULL) {
        return NULL;
    }
    node = queue->head;
    if (node->next == NULL) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = node->next;
    }
    return node;
}

void request_queue_free(request_queue* queue)
{
    request_node* node;
    request_node* temp;

    node = queue->head;
    while (node) {
        temp = node->next;
        request_node_free(node);
        node = temp;
    }
    free(queue);
}

void parse_request_line(request_node* node, char* line_buf)
{
    int n;
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    n = sscanf(line_buf, "%s %s %s", method, uri, version);
    if (n != 3) {
        /* Bad request line */
        node->method = NONE;
        node->res->status_code = BAD_REQUEST;
        node->stage = ABORT;
        node->close = 1;
    } else {
        /* Parse request method */
        if (strstr(method, "GET") == method)
            node->method = GET;
        else if (strstr(method, "POST") == method)
            node->method = POST;
        else if (strstr(method, "HEAD") == method)
            node->method = HEAD;
        else {
            node->method = NONE;
            node->res->status_code = NOT_IMPLEMENTED;
            node->stage = ABORT;
            node->close = 1;
            return;
        }
        /* Parse request version */
        if (strstr(version, "HTTP/1.1") != version) {
            node->res->status_code = VERSION_NOT_SUPPORT;
            node->stage = ABORT;
            node->close = 1;
            return;
        }
        if (strlen(uri) > MAX_PATH_LENGTH - 100) {
            node->res->status_code = URI_TOO_LARGE;
            node->stage = ABORT;
            node->close = 1;
            return;
        }
        strcpy(node->uri, uri);
        node->stage = HEADER;
    }
}

void parse_request_header(request_node* node, char* line_buf)
{
    char token[MAXLINE];
    if (line_buf[0] == '\0') {
        /* \r\n read, current request ends */
        if (node->body_size > 0) {
            /* More message body data need to be received */
            node->stage = BODY;
            return;
        } else {
            /* Finish current request, put into queue */
            if (node->method == POST && node->body_size == -1) {
                node->res->status_code = LENGTH_REQUIRED;
                node->stage = ABORT;
                node->close = 1;
                return;
            }
            /* Request parsing finished */
            node->stage = DONE;
            return;
        }
    }
    node->header_num++;
    if (node->header_num > MAX_HEADER_NUM) {
        node->res->status_code = REQUEST_TOO_LARGE;
        node->stage = ABORT;
        node->close = 1;
        return;
    }
    if (strstr(line_buf, "Connection:") == line_buf) {
        sscanf(line_buf, "Connection: %s", token);
        if (strcmp(token, "close") == 0 ||
                strcmp(token, "Close") == 0)
            node->close = 1;
    } else if (strstr(line_buf, "Content-Length:") == line_buf) {
        sscanf(line_buf, "Content-Length: %d",&(node->body_size));
        if (node->body_size > MAX_BODY_SIZE) {
            node->res->status_code = REQUEST_TOO_LARGE;
            node->stage = ABORT;
            node->close = 1;
            return;
        }
        if (node->body_size < 0) {
            node->res->status_code = BAD_REQUEST;
            node->stage = ABORT;
            node->close = 1;
            return;
        }
    } else if (strstr(line_buf, "Accept:") == line_buf) {
        sscanf(line_buf, "Accept: %[^\r^\n]",
               node->headers.accept);
    } else if (strstr(line_buf, "Accept-Charset:") == line_buf) {
        sscanf(line_buf, "Accept-Charset: %[^\r^\n]",
               node->headers.accept_charset);
    } else if (strstr(line_buf, "Accept-Encoding:") == line_buf) {
        sscanf(line_buf, "Accept-Encoding: %[^\r^\n]",
               node->headers.accept_encoding);
    } else if (strstr(line_buf, "Accept-Language:") == line_buf) {
        sscanf(line_buf, "Accept-Language: %[^\r^\n]",
               node->headers.accept_language);
    } else if (strstr(line_buf, "Content-Type:") == line_buf) {
        sscanf(line_buf, "Content-Type: %[^\r^\n]",
               node->headers.content_type);
    } else if (strstr(line_buf, "Cookie:") == line_buf) {
        sscanf(line_buf, "Cookie: %[^\r^\n]",
               node->headers.cookie);
    } else if (strstr(line_buf, "Host:") == line_buf) {
        sscanf(line_buf, "Host: %[^\r^\n]",
               node->headers.host);
    } else if (strstr(line_buf, "Referer:") == line_buf) {
        sscanf(line_buf, "Referer: %[^\r^\n]",
               node->headers.referer);
    } else if (strstr(line_buf, "User-Agent:") == line_buf) {
        sscanf(line_buf, "User-Agent: %[^\r^\n]",
               node->headers.user_agent);
    } else if (!strchr(line_buf, ':')) {
        node->res->status_code = BAD_REQUEST;
        node->stage = ABORT;
        node->close = 1;
        return;
    }
}

void generate_error_response(request_node *node)
{
    response *res = NULL;
    char body[MAXLINE], status_line[MAXLINE], content_len[MAXLINE];
    response_line* temp_line;

    res = node->res;
    sprintf(body, "<html><title>Server Error</title>");
    if (res->status_code == BAD_REQUEST) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Bad Request");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Bad request, can't parse method or uri or version");
    } else if (res->status_code == NOT_FOUND) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Not Found");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Request content not found");
    } else if (res->status_code == REQUEST_TIME_OUT) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Request Time-out");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Request time out");
    } else if (res->status_code == LENGTH_REQUIRED) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Length Required");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Content-Length required for POST");
    } else if (res->status_code == REQUEST_TOO_LARGE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Request Entity Too Large");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Request too large or request line too long");
    } else if (res->status_code == URI_TOO_LARGE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Request-URI too large");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Request uri larger than server can handle");
    } else if (res->status_code == INTERNAL_SERVER_ERROR) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Internal Server Error");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Some bugs have crawled into the server");
    } else if (res->status_code == NOT_IMPLEMENTED) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Not Implemented");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Method not implemented");
    } else if (res->status_code == SERVICE_UNAVAILABLE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Service unavailable");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Server under maintenance or too many connections");
    } else if (res->status_code == VERSION_NOT_SUPPORT) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "HTTP Version not supported");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "HTTP version not support by server");
    } else {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                res->status_code, "Unknown error");
        sprintf(body, "%s%d: %s\r\n", body, res->status_code,
                "Unknown error");
    }
    sprintf(content_len, "Content-Length: %d\r\n",
            (int)strlen(body));

    temp_line = create_response_line(status_line);
    if (temp_line == NULL) {
        node->close = 1;
        return;
    }
    response_add_line(res, temp_line);
    temp_line = create_response_line(content_len);
    if (temp_line == NULL) {
        node->close = 1;
        return;
    }
    response_add_line(res, temp_line);
    if (node->close) {
        temp_line = create_response_line("Connection: close\r\n");
        if (temp_line == NULL)
            return;
        response_add_line(res, temp_line);
    } else {
        temp_line = create_response_line("Connection: keep-alive\r\n");
        if (temp_line == NULL)
            return;
        response_add_line(res, temp_line);
    }
    temp_line = create_response_line("Content-Type: text/html\r\n");
    if (temp_line == NULL) {
        node->close = 1;
        return;
    }
    response_add_line(res, temp_line);
    temp_line = create_response_line("\r\n");
    if (temp_line == NULL) {
        node->close = 1;
        return;
    }
    response_add_line(res, temp_line);

    res->body_size = strlen(body);
    res->body_buf = (char *)malloc(res->body_size);
    if (res->body_buf == NULL) {
        fprintf(stderr, "No memory to allocate response body\n");
        node->close = 1;
        res->body_size = -1;
        return;
    }
    memcpy(res->body_buf, body, res->body_size);
}
