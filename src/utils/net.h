#ifndef __NET_H__
#define __NET_H__

#include <arpa/inet.h>
#include <netdb.h>

typedef struct sockaddr_in sockaddr_in_t;

/**
 * Construct a struct sockaddr_in using given ip and port
 */
sockaddr_in_t make_sockaddr_in(const char* ip, unsigned int port);

/**
 * Construct a struct addrinfo using given ip and port
 */
struct addrinfo *make_addrinfo(const char* ip, unsigned int port);

#endif
