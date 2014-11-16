#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "request.h"

request_t* create_request(int client_fd) {
    request_t* request = malloc(sizeof(request_t));

    request->client_fd = client_fd;
    request->server_fd = -1;

    request->parse = req_parse;
    request->conenct_server = req_connect_server;
    request->forward = req_forward;

    return request;
}

static int parse_uri(char* str, uri_t *uri)
{
    char *host_start, *path_start, *port_start;
    int prot_len;       //The length of "http://"

    prot_len = strlen("http://");

    if (strstr(str, "http://") == NULL || strlen(str) <= prot_len)
        return -1;

    host_start = str + prot_len;
    path_start = strchr(host_start, '/');
    if (path_start == NULL) {
        strcpy(uri->path, "/");
        strcpy(uri->host, host_start);
    }
    else {
        strcpy(uri->path, path_start);
        strncpy(uri->host, host_start, path_start - host_start);
    }

    port_start = strchr(uri->host, ':');
    if (port_start) {
        uri->port = atoi(port_start + 1);
        uri->host[port_start - uri->host] = '\0';
    } else {
        uri->port = 80;     //Default http port
        if (path_start)
            uri->host[path_start - host_start] = '\0';
    }
    return 0;
}

int req_parse(request_t* self) {
    char uri[REQ_BUF_SIZE];

    if (sscanf(self->buf, "%s %s %s", self->method, uri, self->version) < 3) {
        log_msg(L_ERROR, "Bad request line: %s\n", self->buf);
        return -1;
    }
    if (parse_uri(uri, &self->uri) == -1) {
        log_msg(L_ERROR, "Bad uri: %s\n", uri);
        return -1;
    }

    return 0;
}

int req_connect_server(request_t* self) {

}

int req_forward(request_t* self) {

}
