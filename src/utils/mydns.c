#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "mydns.h"
#include "../utils/net.h"

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
    sendto(sock, node, strlen(node), 0, (struct sockaddr *)&dns_addr, addr_len);

    return 0;
}
