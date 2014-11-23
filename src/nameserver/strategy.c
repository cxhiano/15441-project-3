#include "strategy.h"
#include "globals.h"

int rr_pointer = 0;

char* round_robin(char* qname) {
    rr_pointer = (rr_pointer + 1) % servers->len;
    return servers->get_i(servers, rr_pointer);
}
