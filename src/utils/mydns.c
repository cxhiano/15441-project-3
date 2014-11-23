#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "mydns.h"
#include "net.h"
#include "message.h"

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

static question_t* create_question(const char* qname) {
    question_t* q = create_struct(sizeof(question_t));
    q->QTYPE = q->QCLASS = 1;
    q->QNAME = (char*)qname;

    return q;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res) {
    sockaddr_in_t addr;
    int port = atoi(service);
    int nbytes;
    message_t* msg = create_message();
    char buf[MAXBUF];

    msg->header->QDCOUNT = 1;
    list_add(msg->question, create_question(node));

    nbytes = dumps_message(msg, buf);
    free_message(msg);

    if (sendto(sock, buf, nbytes, 0, (struct sockaddr *)&dns_addr,
            addr_len) == -1) {
        perror("resolve error: sendto()");
        return -1;
    }

    return 0;
}
