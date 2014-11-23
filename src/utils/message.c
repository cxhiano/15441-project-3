#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "message.h"

static void dumps_uint16(char* buf, uint16_t b) {
    buf[0] = b & 0xFF;
    buf[1] = (b >> 8) & 0xFF;
}

void* create_struct(int size) {
    header_t* h = malloc(sizeof(size));
    memset(h, 0, sizeof(size));
    return h;
}

void dumps_header(header_t* h, char* buf) {
    dumps_uint16(buf, h->id);

    buf[2] = (h->QR << 7) + (h->Opcode << 3) + (h->AA << 2) + (h->TC << 1) +
             h->RD;
    buf[3] = (h->RA << 7) + (h->Z << 4) + h->RCODE;


    dumps_uint16(buf + 4, h->QDCOUNT);
    dumps_uint16(buf + 6, h->ANCOUNT);
    dumps_uint16(buf + 8, h->NSCOUNT);
    dumps_uint16(buf + 10, h->ARCOUNT);
}

header_t* loads_header(char* buf) {
    header_t* h = create_struct(sizeof(header_t));

    h->id = *(uint16_t*)buf;

    h->QR = (buf[2] >> 7) & 1;
    h->Opcode = (buf[2] >> 3) & 0xF;
    h->AA = (buf[2] >> 2) & 1;
    h->TC = (buf[2] >> 1) & 1;
    h->RD = buf[2] & 1;

    h->RA = (buf[3] >> 7) & 1;
    h->Z = (buf[3] >> 4) & 7;
    h->RCODE = buf[3] & 0xF;

    h->QDCOUNT = *(((uint16_t*)buf) + 2);
    h->ANCOUNT = *(((uint16_t*)buf) + 3);
    h->NSCOUNT = *(((uint16_t*)buf) + 4);
    h->ARCOUNT = *(((uint16_t*)buf) + 5);

    return h;
}

void dumps_question(question_t* q, char* buf) {
    int len = strlen(q->QNAME) + 1;

    assert(len <= 0xFF);
    buf[0] = len;
    strcpy(buf + 1, q->QNAME);

    dumps_uint16(buf + len + 1, q->QTYPE);
    dumps_uint16(buf + len + 3, q->QCLASS);
}

question_t* loads_question(char* buf) {
    question_t* q = create_struct(sizeof(question_t));
    int len = buf[0];

    q->QNAME = malloc(len);
    strcpy(q->QNAME, buf + 1);

    q->QTYPE = *((uint16_t*)(buf + len + 1));
    q->QCLASS = *((uint16_t*)(buf + len + 3));

    return q;
}
