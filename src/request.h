#ifndef __REQUEST_H__
#define __REQUEST_H__

#define REQ_BUF_SIZE 8196

#define S_REQ_LINE 0
#define S_HEADERS 1
#define S_REQ_END 2

typedef struct {
    char host[REQ_BUF_SIZE], path[REQ_BUF_SIZE];
    int port;
} uri_t;

typedef struct request_s request_t;
struct request_s {
    int client_fd;
    int server_fd;
    char buf[REQ_BUF_SIZE];
    char method[REQ_BUF_SIZE], version[REQ_BUF_SIZE];
    uri_t uri;

    int (*parse)(request_t* self);

    int (*connect_server)(request_t* self);

    int (*forward)(request_t* self);
};

request_t* create_request();

int req_parse(request_t*);
int req_connect_server(request_t*);
int req_forward(request_t*);

#endif
