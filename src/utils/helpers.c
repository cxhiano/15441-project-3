#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "net.h"
#include "list.h"
#include "message.h"
#include "helpers.h"

int dumps_request(int id, char* domain, char* buf) {
    question_t* q = create_struct(sizeof(question_t));
    message_t* msg = create_message();
    int nbytes;

    msg->header->id = id;
    msg->header->QDCOUNT = 1;
    q->QTYPE = q->QCLASS = 1;
    q->QNAME = domain;
    list_add(msg->question, q);

    nbytes = dumps_message(msg, buf);

    free_message(msg);

    return nbytes;
}

int dumps_response(message_t* msg, char* ip, char* buf) {
    question_t* q = list_get(msg->question, 0);
    resource_t* r = create_struct(sizeof(resource_t));
    int nbytes;
    sockaddr_in_t addr;

    msg->header->AA = 1;

    if (ip == NULL) {
        msg->header->RCODE = 3;
        free(r);
    } else {
        msg->header->ANCOUNT = 1;
        r->NAME = q->QNAME;
        r->TYPE = r->CLASS = 1;
        r->TTL = 0;
        r->RDLENGTH = 4;
        inet_aton(ip, &addr.sin_addr);

        r->RDATA = malloc(4);
        r->RDATA[0] = addr.sin_addr.s_addr & 0xFF;
        r->RDATA[1] = (addr.sin_addr.s_addr >> 8) & 0xFF;
        r->RDATA[2] = (addr.sin_addr.s_addr >> 16) & 0xFF;
        r->RDATA[3] = (addr.sin_addr.s_addr >> 24) & 0xFF;

        list_add(msg->answer, r);
    }

    nbytes = dumps_message(msg, buf);

    return nbytes;
}

char* loads_response(char* buf) {
    message_t* msg = loads_message(buf);
    resource_t* r = list_get(msg->answer, 0);
    sockaddr_in_t addr;
    char *ip;

    if (r == NULL) {
        free_message(msg);
        return NULL;
    }

    addr.sin_addr.s_addr = (r->RDATA[0] & 255) + (r->RDATA[1] << 8) +
                           (r->RDATA[2] << 16) + (r->RDATA[3] << 24);
    free_message(msg);

    ip = inet_ntoa(addr.sin_addr);

    return ip;
}
