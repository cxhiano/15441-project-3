#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "globals.h"
#include "strategy.h"
#include "../utils/log.h"
#include "../utils/net.h"
#include "../utils/message.h"
#include "../utils/helpers.h"

#define MAXBUF 8192

int now()
{
    struct timeval now;

    gettimeofday(&now, NULL);
    return now.tv_sec;
}

void usage() {
    puts("./nameserver [-r] <log> <ip> <port> <servers> <LSAs>\n");
}

/**
 * Setup socket listen on given ip and port
 * @return      0 if socket is setup successfully. Otherwise -1
 */
int setup_dns_server(char* ip, int port) {
    int sock;
    struct sockaddr_in myaddr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        log_error("setup_dns_server socket()");
        return -1;
    }

    myaddr = make_sockaddr_in(ip, port);

    if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        log_error("setup_dns_server bind()");
        return -1;
    }

    return sock;
}

/**
 * Parse server list from given file
 * @return       0 on success. Otherwise -1
 */
int get_server_list(char* fname) {
    FILE* fp;
    char server_ip[32];
    char* tmp;

    if ((fp = fopen(fname, "r")) == NULL) {
        log_error("get_server_list: fopen() error");
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

/**
 * Listen on given file descriptor. Receive, parse and respnose DNS queries.
 */
void serve(int listen_fd) {
    char buf[MAXBUF];
    sockaddr_in_t from;
    socklen_t addr_len = sizeof(from);
    ssize_t nbytes;
    char *domain, *ip, *from_ip;

    while (1) {
        nbytes = recvfrom(listen_fd, buf, MAXBUF, 0, (struct sockaddr*)&from,
                          &addr_len);

        if (nbytes < 0) {
            log_error("serve: recv() error");
            continue;
        } else {
            // Deserialize request
            domain = loads_request(buf);

            from_ip = inet_ntoa(from.sin_addr);

            if (domain != NULL) {
                // Get ip of requested domain
                if (strategy == S_ROUND_ROBIN)
                    ip = round_robin(domain);
                else
                    ip = nearest_server(domain, from_ip);
            }

            // The request contains no question or the request domain is not
            // video.cs.cmu.edu
            if (domain == NULL || ip == NULL ||
                    strcmp(domain, "video.cs.cmu.edu") != 0) {
                log_msg(L_DEBUG, "Invalid request from %s\n", from_ip);
                nbytes = dumps_response(NULL, NULL, buf);
            } else {
                log_msg(L_INFO, "%d %s %s %s\n", now(), from_ip, domain, ip);

                // Serialize response
                nbytes = dumps_response(domain, ip, buf);
            }


            if (sendto(listen_fd, buf, nbytes, 0, (struct sockaddr *)&from,
                    addr_len) == -1)
                log_error("serve: sendto() error");

            if (domain) free(domain);
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

    log_mask = L_ERROR | L_INFO | L_DEBUG;
    set_log_file(argv[1]);
    if ((listen_fd = setup_dns_server(argv[i + 1], atoi(argv[i + 2]))) == -1)
        exit(1);
    if (get_server_list(argv[i + 3]) == -1)
        exit(1);
    if (lsa_init(argv[i + 4]) == -1)
        exit(1);

    serve(listen_fd);

    return 0;
}
