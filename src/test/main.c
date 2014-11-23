#include "../utils/mydns.h"
#include "../utils/message.h"

int main() {
    char* buf;
    header_t* tmp = create_header();

    tmp->id = 1;
    tmp->QR = 1;
    tmp->AA = 1;
    tmp->TC = 1;
    tmp->RD = 1;
    tmp->RA = 1;
    tmp->Z = 1;
    tmp->RCODE = 1;
    tmp->QDCOUNT = 1;
    tmp->ANCOUNT = 2;
    tmp->NSCOUNT = 3;
    tmp->ARCOUNT = 4;

    buf = dumps_header(tmp);
    tmp = loads_header(buf);

    return 0;
}
