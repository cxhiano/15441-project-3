#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "message.h"

static void dumps_uint16(char* buf, uint16_t b) {
    buf[0] = b & 0xFF;
    buf[1] = (b >> 8) & 0xFF;
}

static inline uint16_t loads_uint16(char* buf) {
    return *(uint16_t*)buf;
}

static void dumps_uint32(char* buf, uint32_t b) {
    buf[0] = b & 0xFF;
    buf[1] = (b >> 8) & 0xFF;
    buf[2] = (b >> 16) & 0xFF;
    buf[3] = (b >> 24) & 0xFF;
}

static inline uint32_t loads_uint32(char* buf) {
    return *(uint32_t*)buf;
}

static void dumps_string(char* buf, char* str, int len) {
    strncpy(buf, str, len);
}

static char* loads_string(char* buf, int len) {
    char* name = malloc(buf[0]);
    strncpy(name, buf, len);
    return name;
}


void* create_struct(int size) {
    void* s = malloc(size);
    memset(s, 0, size);
    return s;
}

int dumps_header(header_t* h, char* buf) {
    dumps_uint16(buf, h->id);

    buf[2] = (h->QR << 7) + (h->Opcode << 3) + (h->AA << 2) + (h->TC << 1) +
             h->RD;
    buf[3] = (h->RA << 7) + (h->Z << 4) + h->RCODE;


    dumps_uint16(buf + 4, h->QDCOUNT);
    dumps_uint16(buf + 6, h->ANCOUNT);
    dumps_uint16(buf + 8, h->NSCOUNT);
    dumps_uint16(buf + 10, h->ARCOUNT);

    return HEADER_SIZE;
}

header_t* loads_header(char* buf) {
    header_t* h = create_struct(sizeof(header_t));

    h->id = loads_uint16(buf);

    h->QR = (buf[2] >> 7) & 1;
    h->Opcode = (buf[2] >> 3) & 0xF;
    h->AA = (buf[2] >> 2) & 1;
    h->TC = (buf[2] >> 1) & 1;
    h->RD = buf[2] & 1;

    h->RA = (buf[3] >> 7) & 1;
    h->Z = (buf[3] >> 4) & 7;
    h->RCODE = buf[3] & 0xF;

    h->QDCOUNT = loads_uint16(buf + 4);
    h->ANCOUNT = loads_uint16(buf + 6);
    h->NSCOUNT = loads_uint16(buf + 8);
    h->ARCOUNT = loads_uint16(buf + 10);

    return h;
}

int dumps_question(question_t* q, char* buf) {
    int len = strlen(q->QNAME) + 1;

    buf[0] = len;
    dumps_string(buf + 1, q->QNAME, len);

    dumps_uint16(buf + len + 1, q->QTYPE);
    dumps_uint16(buf + len + 3, q->QCLASS);

    return len + 5;
}

question_t* loads_question(char* buf) {
    question_t* q = create_struct(sizeof(question_t));
    int len = (unsigned char)buf[0];

    q->QNAME = loads_string(buf + 1, len);

    q->QTYPE = loads_uint16(buf + len + 1);
    q->QCLASS = loads_uint16(buf + len + 3);

    q->size = len + 5;

    return q;
}

int dumps_resource(resource_t* r, char* buf) {
    int len = strlen(r->NAME);

    buf[0] = len;
    dumps_string(buf + 1, r->NAME, len);

    dumps_uint16(buf + len + 1, r->TYPE);
    dumps_uint16(buf + len + 3, r->CLASS);
    dumps_uint32(buf + len + 5, r->TTL);
    dumps_uint16(buf + len + 9, r->RDLENGTH);
    dumps_string(buf + len + 11, r->RDATA, r->RDLENGTH);

    return len + 11 + r->RDLENGTH;
}

resource_t* loads_resource(char* buf) {
    resource_t* r = create_struct(sizeof(resource_t));
    int len = (unsigned char)buf[0];

    r->NAME = loads_string(buf + 1, len);

    r->TYPE = loads_uint16(buf + len + 1);
    r->CLASS = loads_uint16(buf + len + 3);
    r->TTL = loads_uint32(buf + len + 5);
    r->RDLENGTH = loads_uint16(buf + len + 9);
    r->RDATA = loads_string(buf + len + 11, r->RDLENGTH);

    r->size = len + 11 + r->RDLENGTH;

    return r;
}

message_t* create_message() {
    message_t* msg = create_struct(sizeof(message_t));
    msg->header = create_struct(sizeof(header_t));
    msg->question = create_list();
    msg->answer = create_list();
    msg->authority = create_list();
    msg->additional = create_list();
    return msg;
}

int dumps_message(message_t* msg, char* buf) {
    assert(msg->header->QDCOUNT == msg->question->len);
    assert(msg->header->ANCOUNT == msg->answer->len);
    assert(msg->header->NSCOUNT == msg->authority->len);
    assert(msg->header->ARCOUNT == msg->additional->len);

    int bytes_dumped = 0;
    item_t* item;
    question_t* q;
    resource_t* r;

    bytes_dumped += dumps_header(msg->header, buf);

    ITER_LIST(item, msg->question) {
        q = item->content;
        bytes_dumped += dumps_question(q, buf + bytes_dumped);
    }

    ITER_LIST(item, msg->answer) {
        r = item->content;
        bytes_dumped += dumps_resource(r, buf + bytes_dumped);
    }

    ITER_LIST(item, msg->authority) {
        r = item->content;
        bytes_dumped += dumps_resource(r, buf + bytes_dumped);
    }

    ITER_LIST(item, msg->additional) {
        r = item->content;
        bytes_dumped += dumps_resource(r, buf + bytes_dumped);
    }

    return bytes_dumped;
}

message_t* loads_message(char* buf) {
    int bytes_loaded = 0;
    int i;
    question_t* q;
    resource_t* r;
    message_t* msg = create_struct(sizeof(message_t));

    msg->header = loads_header(buf);
    bytes_loaded += HEADER_SIZE;

    msg->question = create_list();
    for (i = 0; i < msg->header->QDCOUNT; ++i) {
        q = loads_question(buf + bytes_loaded);
        bytes_loaded += q->size;
        list_add(msg->question, q);
    }

    msg->answer = create_list();
    for (i = 0; i < msg->header->ANCOUNT; ++i) {
        r = loads_resource(buf + bytes_loaded);
        bytes_loaded += r->size;
        list_add(msg->answer, r);
    }

    msg->authority = create_list();
    for (i = 0; i < msg->header->NSCOUNT; ++i) {
        r = loads_resource(buf + bytes_loaded);
        bytes_loaded += r->size;
        list_add(msg->authority, r);
    }

    msg->additional = create_list();
    for (i = 0; i < msg->header->ARCOUNT; ++i) {
        r = loads_resource(buf + bytes_loaded);
        bytes_loaded += r->size;
        list_add(msg->additional, r);
    }

    return msg;
}
