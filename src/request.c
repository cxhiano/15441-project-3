#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "request.h"
#include "config.h"

request_t* create_request(int client_fd) {
    request_t* request = malloc(sizeof(request_t));

    request->client_fd = client_fd;
    request->server_fd = -1;

    request->parse = req_parse;
    request->connect_server = req_connect_server;
    request->forward = req_forward;
    request->finalize = req_finalize;

    return request;
}

static int parse_uri(char* str, uri_t *uri)
{
    char *host_start, *path_start, *port_start;
    int prot_len;       //The length of "http://"

    if (str[0] == '/') {
        strcpy(uri->host, "localhost");
        strcpy(uri->path, str);
    } else {
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

static sockaddr_in_t make_sockaddr_in(char* ip, int port) {
    sockaddr_in_t addr;

    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    return addr;
}

int req_connect_server(request_t* self) {
    sockaddr_in_t serveraddr, clientaddr;

    if ((self->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("req_connect_server: socket() failed");
        return -1;
    }

    clientaddr = make_sockaddr_in(fake_ip, 0);
    if (bind(self->server_fd, (struct sockaddr *)&clientaddr,
            sizeof(clientaddr))) {
        log_error("req_connect_server: Failed binding socket");
        return -1;
    }

    serveraddr = make_sockaddr_in(www_ip, self->uri.port);

    if (connect(self->server_fd, (struct sockaddr *) &serveraddr,
            sizeof(serveraddr)) < 0) {
        log_error("req_connect_server: connect() failed");
        return -1;
    }

    return 0;
}

int req_forward(request_t* self, int from_fd, int to_fd) {
    int nbytes;

    nbytes = read(from_fd, self->buf, REQ_BUF_SIZE);

    if (nbytes == -1) {
        log_error("req_forward: read error");
        return -1;
    }

    if (write(to_fd, self->buf, nbytes) == -1) {
        log_error("req_forward: write error");
        return -1;
    }

    return nbytes;
}

void req_finalize(request_t* self) {
    close(self->client_fd);
    if (self->server_fd)
        close(self->server_fd);
}
