#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "log.h"
#include "proxy.h"
#include "../utils/mydns.h"

int main(int argc, char* argv[])
{
    int listen_port, dns_port;
    double alpha;
    char fake_ip[32];
    char dns_ip[32];
    char www_ip[32];
    struct addrinfo *lookup = NULL;
    struct sockaddr_in *addr;

    /* Parse arguments */
    if (argc < 7) {
        fprintf(stdout,
                "usage: %s <log> <alpha> <listen-port> <fake-ip> <dns-ip> "
                "<dns-port> [<www-ip>]",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    log_initialize(argv[1]);
    /* Initialize DNS server settings */
    if (strlen(argv[5]) > 31) {
        fprintf(stdout, "DNS IP too long: %s\n", argv[5]);
        exit(EXIT_FAILURE);
    }
    strcpy(dns_ip, argv[5]);
    dns_port = atoi(argv[6]);
    if (dns_port < 0 || dns_port > 65535) {
        fprintf(stdout, "Invalid DNS port: %s\n", argv[6]);
        exit(EXIT_FAILURE);
    }
    /* Initialize proxy server */
    alpha = atof(argv[2]);
    if (alpha < 0 || alpha > 1) {
        fprintf(stdout, "Invalid alpha: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    listen_port = atoi(argv[3]);
    if (listen_port < 0 || listen_port > 65535) {
        fprintf(stdout, "Invalid listen port: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }
    if (strlen(argv[4]) > 31) {
        fprintf(stdout, "Fake IP too long: %s\n", argv[4]);
        exit(EXIT_FAILURE);
    }
    strcpy(fake_ip, argv[4]);
    init_mydns(dns_ip, dns_port, fake_ip);
    if (argc > 7) {
        if (strlen(argv[7]) > 31) {
            fprintf(stdout, "Fake IP too long: %s\n", argv[7]);
            exit(EXIT_FAILURE);
        }
        strcpy(www_ip, argv[7]);
    } else {
        if (resolve("video.cs.cmu.edu", SERVER_PORT_STR, NULL, &lookup) == 0) {
            addr = (struct sockaddr_in *)lookup->ai_addr;
            strcpy(www_ip, inet_ntoa(addr->sin_addr));
        } else {
            fprintf(stdout, "Cannot resolve video.cs.cmu.edu\n");
            exit(EXIT_FAILURE);
        }
        free(lookup);
    }
    proxy_initialize(www_ip, fake_ip, alpha);
    run_server(listen_port);

    return EXIT_SUCCESS;
}
