#include "../utils/list.h"

#define S_ROUND_ROBIN 0
#define S_LSA 1

// DNS load balancing strategy
int strategy;

#define SERVER_PORT 8080

// A list of video servers
list_t* servers;
