#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "globals.h"
#include "strategy.h"
#include "../utils/net.h"
#include "../utils/message.h"
#include "../utils/helpers.h"

#define MAXBUF 8192

void usage() {
    puts("./nameserver [-r] <log> <ip> <port> <servers> <LSAs>\n");
}

int setup_dns_server(char* ip, int port) {
    int sock;
    struct sockaddr_in myaddr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("setup_dns_server socket()");
        return -1;
    }

    myaddr = make_sockaddr_in(ip, port);

    if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        perror("setup_dns_server bind()");
        return -1;
    }

    return sock;
}

int get_server_list(char* fname) {
    FILE* fp;
    char server_ip[32];
    char* tmp;

    if ((fp = fopen(fname, "r")) == NULL) {
        perror("get_server_list: fopen() error");
        return -1;
    }

    servers = create_list();

    while (fscanf(fp, "%s", server_ip) != EOF) {
        tmp = malloc(32);
        strcpy(tmp, server_ip);
        servers->add(servers, tmp);
    }

    return 0;
}

resource_t* create_answer(char* domain, char* ip) {
    resource_t* r = create_struct(sizeof(resource_t));
    r->NAME = domain;
    r->TYPE = r->CLASS = 1;
    r->TTL = 0;
    r->RDLENGTH = strlen(ip);
    r->RDATA = ip;

    return r;
}

void serve(int listen_fd) {
    char buf[MAXBUF];
    sockaddr_in_t from;
    socklen_t addr_len = sizeof(from);
    ssize_t nbytes;
    char *domain, *ip;

    while (1) {
        nbytes = recvfrom(listen_fd, buf, MAXBUF, 0, (struct sockaddr*)&from,
                          &addr_len);

        if (nbytes < 0) {
            perror("serve: recv() error");
            continue;
        } else {
            domain = loads_request(buf);
            ip = round_robin(domain);

            nbytes = dumps_response(domain, ip, buf);

            if (sendto(listen_fd, buf, nbytes, 0, (struct sockaddr *)&from,
                    addr_len) == -1)
                perror("serve: sendto() error");

            free(domain);
        }
    }

}

int main(int argc, char* argv[]) {
    int i;
    int listen_fd;

    if (argc < 2) {
        usage();
        exit(0);
    }

    if (strcmp(argv[1], "-r") == 0) {
        strategy = S_ROUND_ROBIN;
        if (argc < 7) {
            usage();
            exit(0);
        }
        i = 2;
    } else {
        strategy = S_LSA;
        if (argc < 6) {
            usage();
            exit(0);
        }
        i = 1;
    }

    if ((listen_fd = setup_dns_server(argv[i + 1], atoi(argv[i + 2]))) == -1)
        exit(1);
    if (get_server_list(argv[i + 3]) == -1)
        exit(1);

    serve(listen_fd);

    return 0;
}
