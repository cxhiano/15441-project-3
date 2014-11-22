#include <arpa/inet.h>
#include <string.h>
#include "net.h"

sockaddr_in_t make_sockaddr_in(const char* ip, unsigned int port) {
    sockaddr_in_t addr;

    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    return addr;
}
