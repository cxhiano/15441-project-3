#include <stdio.h>
#include <string.h>
#include "strategy.h"
#include "globals.h"
#include "graph.h"
#include "../utils/list.h"

int rr_pointer = 0;

char* round_robin(char* qname) {
    rr_pointer = (rr_pointer + 1) % servers->len;
    return servers->get_i(servers, rr_pointer);
}

graph_t* G;
#define MAXBUF 8192

list_t* parse_neighbors(char* neighbors) {
    char* token = strtok(neighbors, ",");
    list_t* list = create_list();

    while (token) {
        list->add(list, token);
        token = strtok(NULL, ",");
    }

    return list;
}

int lsa_init(char* lsa_file) {
    char neighbors[MAXBUF];
    char sender[MAXBUF];
    int ts;
    FILE* fp;
    node_t* node;

    if ((fp = fopen(lsa_file, "r")) == NULL) {
        perror("lsa_init: fopen() error");
        return -1;
    }

    G = create_graph();

    while (fscanf(fp, "%s %d %s", sender, &ts, neighbors) != EOF) {
        node = get_node_by_name(G, sender);
        if (node == NULL)
            node = create_node(G, sender);
        update_node(node, ts, parse_neighbors(neighbors));
    }

    return 0;
}

void print_graph() {
    node_t *n1, *n2;
    item_t *i1, *i2;

    ITER_LIST(i1, G->nodes) {
        n1 = i1->content;
        printf("%s:", n1->name);
        ITER_LIST(i2, n1->neighbors) {
            n2 = i2->content;
            printf(" %s", n2->name);
        }
        puts("");
    }
}
