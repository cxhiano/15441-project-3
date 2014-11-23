#include <stdlib.h>
#include <string.h>
#include "net.h"

sockaddr_in_t make_sockaddr_in(const char* ip, unsigned int port) {
    sockaddr_in_t addr;

    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    return addr;
}

struct addrinfo *make_addrinfo(const char* ip, unsigned int port) {
    struct addrinfo *addr;
    sockaddr_in_t sockaddr;

    addr = malloc(sizeof(struct addrinfo));
    memset(addr, 0, sizeof(struct addrinfo));
    addr->ai_family = AF_INET;
    addr->ai_socktype = SOCK_STREAM;
    addr->ai_protocol = 0;
    addr->ai_addrlen = sizeof(sockaddr_in_t);
    addr->ai_addr = malloc(addr->ai_addrlen);
    sockaddr = make_sockaddr_in(ip, port);
    memcpy(addr->ai_addr, &sockaddr, addr->ai_addrlen);
    addr->ai_canonname = NULL;
    addr->ai_next = NULL;

    return addr;
}
