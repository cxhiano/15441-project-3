#include "../utils/list.h"

#define S_ROUND_ROBIN 0
#define S_LSA 1

int strategy;

#define SERVER_PORT 8080

list_t* servers;
