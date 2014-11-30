#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "proxy.h"
#include "response.h"
#include "../utils/net.h"

char *www_ip = NULL;
char *fake_ip = NULL;
double alpha = 0;

void proxy_initialize(char *new_www_ip, char *new_fake_ip, double new_alpha)
{
    if (www_ip == NULL)
        www_ip = (char *)malloc(strlen(new_www_ip)+1);
    if (www_ip == NULL) {
        fprintf(stderr, "No memory to record www ip");
        exit(EXIT_FAILURE);
    }
    strcpy(www_ip, new_www_ip);
    if (fake_ip == NULL)
        fake_ip = (char *)malloc(strlen(new_fake_ip)+1);
    if (fake_ip == NULL) {
        fprintf(stderr, "No memory to record fake ip");
        exit(EXIT_FAILURE);
    }
    strcpy(fake_ip, new_fake_ip);
    alpha = new_alpha;
}

void handle_proxy_request(connection *conn)
{
    request_node* node;

    
    if (conn->proxy_fd == -1) {
        connect_to_server(conn);
        if (conn->close)
            return;
    }
    node = conn->queue->head;
    while (node) {
        if (node->stage == ABORT || node->stage == DONE) {
            node->res->status_code = SERVICE_UNAVAILABLE;
            generate_error_response(node);
            node->stage = READY;
        } else if (node->stage == DONE) {
            //TODO Modifdy request and forward to server
            //parse_node_uri(node);
        }
        if (node->close)
            break;
        node = node->next;
    }
}

void connect_to_server(connection *conn)
{
    sockaddr_in_t serveraddr, clientaddr;

    if ((conn->proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Fail to create socket\n");
        conn->close = 1;
        return;
    }
    clientaddr = make_sockaddr_in(fake_ip, 0);
    if (bind(conn->proxy_fd, (struct sockaddr *)&clientaddr,
             sizeof(clientaddr))) {
        fprintf(stderr, "Fail to bind to fake ip\n");
        close(conn->proxy_fd);
        conn->proxy_fd = -1;
        conn->close = 1;
        return;
    }
    serveraddr = make_sockaddr_in(www_ip, SERVER_PORT);
    if (connect(conn->proxy_fd, (struct sockaddr *) &serveraddr,
                sizeof(serveraddr)) < 0) {
        fprintf(stderr, "Fail to bind to www ip\n");
        close(conn->proxy_fd);
        conn->proxy_fd = -1;
        conn->close = 1;
        return;
    }
}
