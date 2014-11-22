#ifndef __NET_H__
#define __NET_H__

typedef struct sockaddr_in sockaddr_in_t;

sockaddr_in_t make_sockaddr_in(const char* ip, unsigned int port);

#endif
