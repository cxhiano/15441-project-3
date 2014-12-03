#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "net.h"
#include "list.h"
#include "message.h"
#include "helpers.h"

int dumps_request(char* domain, char* buf) {
    question_t* q = create_struct(sizeof(question_t));
    message_t* msg = create_message();
    int nbytes;

    msg->header->QDCOUNT = 1;
    q->QTYPE = q->QCLASS = 1;
    q->QNAME = domain;
    list_add(msg->question, q);

    nbytes = dumps_message(msg, buf);

    free_message(msg);

    return nbytes;
}

char* loads_request(char* buf) {
    message_t* msg = loads_message(buf);
    question_t* r = list_get(msg->question, 0);
    char* domain;

    if (r == NULL) {
        free_message(msg);
        return NULL;
    }

    domain = r->QNAME;
    free_message(msg);

    return domain;
}

int dumps_response(char* domain, char* ip, char* buf) {
    resource_t* r = create_struct(sizeof(resource_t));
    message_t* msg = create_message();
    int nbytes;
    sockaddr_in_t addr;

    msg->header->AA = 1;
    if (domain == NULL) {
        msg->header->RCODE = 3;
        free(r);
    } else {
        msg->header->ANCOUNT = 1;

        r->NAME = domain;
        r->TYPE = r->CLASS = 1;
        r->TTL = 0;
        r->RDLENGTH = 4;
        inet_aton(ip, &addr.sin_addr);
        r->RDATA = malloc(4);
        dumps_uint32(r->RDATA, addr.sin_addr.s_addr);
        list_add(msg->answer, r);
    }

    nbytes = dumps_message(msg, buf);
    free_message(msg);

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

    addr.sin_addr.s_addr = loads_uint32(r->RDATA);
    free_message(msg);

    ip = inet_ntoa(addr.sin_addr);

    return ip;
}
