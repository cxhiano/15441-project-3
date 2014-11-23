#include <stdio.h>
#include "../utils/net.h"
#include "../utils/mydns.h"
#include "../utils/message.h"

#define BUFSIZE 8192

char buf[BUFSIZE];

void test_message() {
    question_t* q = create_struct(sizeof(question_t));
    resource_t* r = create_struct(sizeof(resource_t));
    message_t* msg = create_message();

    msg->header->id = 1123;
    msg->header->QR = 1;
    msg->header->AA = 1;
    msg->header->TC = 1;
    msg->header->RD = 1;
    msg->header->RA = 1;
    msg->header->Z = 1;
    msg->header->RCODE = 1;
    msg->header->QDCOUNT = 1;
    msg->header->ANCOUNT = 1;
    msg->header->NSCOUNT = 0;
    msg->header->ARCOUNT = 0;

    q->QNAME = "video.cs.cmu.edu";
    q->QTYPE = 1;
    q->QCLASS = 1;

    r->NAME = "video.cs.cmu.edu";
    r->TYPE = 1;
    r->CLASS = 1;
    r->TTL = 214722222;
    r->RDLENGTH = 8;
    r->RDATA = "3.0.0.1";

    list_add(msg->question, q);
    list_add(msg->answer, r);

    printf("%d\n", dumps_message(msg, buf));
    msg = loads_message(buf);

}

void test_dns() {
    init_mydns("127.0.0.1", 12345);
    resolve("video.cs.cmu.edu", "8080", NULL, NULL);

}

int main() {
    test_dns();
    return 0;
}
