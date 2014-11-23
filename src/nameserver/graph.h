#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "../utils/list.h"

typedef struct node_s node_t;
typedef struct graph_s graph_t;

struct node_s {
    char* name;
    graph_t* G;
    int ts;
    list_t* neighbors;
};

node_t* create_node(graph_t* G, char* name);
void free_node(node_t* node);
void add_neighbor(node_t* node, node_t* neighbor);
void update_node(node_t* node, int ts, list_t* neighbor_name);

struct graph_s {
    list_t* nodes;
};

graph_t* create_graph();
void free_graph(graph_t* G);
void add_node(graph_t* G, node_t* n);
node_t* get_node_by_name(graph_t* G, char* name);

#endif
