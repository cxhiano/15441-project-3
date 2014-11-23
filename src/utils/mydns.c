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
int addr_len = sizeof(sockaddr_in_t);
sockaddr_in_t  dns_addr;

int init_mydns(const char *dns_ip, unsigned int dns_port) {
    dns_addr = make_sockaddr_in(dns_ip, dns_port);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("init_mydns socket()");
        return -1;
    }

    return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res) {
    char* ip;
    int port = atoi(service);
    int nbytes;
    char buf[MAXBUF];

    nbytes = dumps_request((char*)node, buf);

    if (sendto(sock, buf, nbytes, 0, (struct sockaddr *)&dns_addr,
            addr_len) == -1) {
        perror("resolve error: sendto()");
        return -1;
    }

    nbytes = recv(sock, buf, MAXBUF, 0);
    ip = loads_response(buf);
    if (ip == NULL) return -1;

    *res = make_addrinfo(ip, port);
    free(ip);

    return 0;
}
