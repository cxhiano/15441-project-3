#ifndef __REQUEST_H__
#define __REQUEST_H__

#define REQ_BUF_SIZE 8196

typedef struct request_s request_t;
struct request_s {
    int client_fd;
    int server_fd;
    int req_content_len;
    char* buf[REQ_BUF_SIZE];
};

#endif
