#include <stdlib.h>
#include <string.h>
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
    char* domain = r->QNAME;

    free_message(msg);

    return domain;
}

int dumps_response(char* domain, char* ip, char* buf) {
    resource_t* r = create_struct(sizeof(resource_t));
    message_t* msg = create_message();
    int nbytes;

    msg->header->AA = 1;
    msg->header->ANCOUNT = 1;

    r->NAME = domain;
    r->TYPE = r->CLASS = 1;
    r->TTL = 0;
    r->RDLENGTH = strlen(ip) + 1;
    r->RDATA = ip;
    list_add(msg->answer, r);

    nbytes = dumps_message(msg, buf);

    free_message(msg);

    return nbytes;
}

char* loads_response(char* buf) {
    message_t* msg = loads_message(buf);
    resource_t* r = list_get(msg->answer, 0);
    char* ip = r->RDATA;

    free_message(msg);

    return ip;
}
