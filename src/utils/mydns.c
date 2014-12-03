#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "mydns.h"
#include "log.h"
#include "net.h"
#include "message.h"
#include "helpers.h"

#define MAXBUF 8192

int sock;
int id;
int addr_len = sizeof(sockaddr_in_t);
sockaddr_in_t  dns_addr;

int init_mydns(const char *dns_ip, unsigned int dns_port, char* client_ip) {
    sockaddr_in_t clientaddr;

    dns_addr = make_sockaddr_in(dns_ip, dns_port);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        log_error("init_mydns socket()");
        return -1;
    }

    clientaddr = make_sockaddr_in(client_ip, 0);
    if (bind(sock, (struct sockaddr *)&clientaddr, sizeof(clientaddr))) {
        log_error("init_mydns: bind() error");
        return -1;
    }

    id = 0;

    return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res) {
    char* ip;
    int port = atoi(service);
    int nbytes;
    char buf[MAXBUF];
    int i;

    nbytes = dumps_request(id++, (char*)node, buf);

    if (sendto(sock, buf, nbytes, 0, (struct sockaddr *)&dns_addr,
            addr_len) == -1) {
        log_error("resolve error: sendto()");
        return -1;
    }

    nbytes = recv(sock, buf, MAXBUF, 0);
    for (i = 0; i < nbytes; ++i) {
        log_msg(L_ERROR, "%x ", buf[i] & 255);
        if (i % 2 == 1)
            log_msg(L_DEBUG, "\n");
    }
    ip = loads_response(buf);
    if (ip == NULL) return -1;
    log_msg(L_DEBUG, "%s\n", ip);

    *res = make_addrinfo(ip, port);

    return 0;
}
