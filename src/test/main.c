#include "../utils/mydns.h"
#include "../utils/message.h"

#define BUFSIZE 8192

void test_header() {
    char buf[BUFSIZE];
    header_t* h = create_struct(sizeof(header_t));

    h->id = 1;
    h->QR = 1;
    h->AA = 1;
    h->TC = 1;
    h->RD = 1;
    h->RA = 1;
    h->Z = 1;
    h->RCODE = 1;
    h->QDCOUNT = 1;
    h->ANCOUNT = 2;
    h->NSCOUNT = 3;
    h->ARCOUNT = 4;

    dumps_header(h, buf);
    h = loads_header(buf);
}

void test_question() {
    char buf[BUFSIZE];
    question_t* q = create_struct(sizeof(question_t));

    q->QNAME = "video.cs.cmu.edu";
    q->QTYPE = 1;
    q->QCLASS = 1;

    dumps_question(q, buf);
    q = loads_question(buf);
}

int main() {
    test_question();
    return 0;
}
