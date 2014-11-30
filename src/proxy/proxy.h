#ifndef PROXY_H
#define PROXY_H

#include "server.h"

#define SERVER_PORT 8080
#define SERVER_PORT_STR "8080"

/**
 * Initialize the proxy with the given www ip, fake ip and alpha.
 */
void proxy_initialize(char *new_www_ip, char *new_fake_ip, double new_alpha);
/**
 * Handle a connection's request by alter its uri and forward to the server
 */
void handle_proxy_request(connection *conn);
/** 
 * Establish a "pipe" between client and server
 */
void connect_to_server(connection *conn);

#endif
