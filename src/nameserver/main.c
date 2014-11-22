#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "globals.h"
#include "../utils/net.h"

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

    return 0;
}


int get_server_list(char* fname) {
    FILE* fp;
    char server_ip[32];
    sockaddr_in_t* addr;

    if ((fp = fopen(fname, "r")) == NULL) {
        perror("get_server_list: fopen() error");
        return -1;
    }

    servers = create_list();

    while (fscanf(fp, "%s", server_ip) != EOF) {
        addr = malloc(sizeof(sockaddr_in_t));
        *addr = make_sockaddr_in(server_ip, SERVER_PORT);
        servers->add(servers, addr);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int i;

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

    if (setup_dns_server(argv[i + 1], atoi(argv[i + 2])) == -1)
        exit(1);
    if (get_server_list(argv[i + 3]) == -1)
        exit(1);

    return 0;
}
