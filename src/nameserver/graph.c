#include <string.h>
#include <stdlib.h>
#include "graph.h"

node_t* create_node(graph_t* G, char* name) {
   node_t* node = malloc(sizeof(node_t));

   node->name = malloc(strlen(name) + 1);
   strcpy(node->name, name);

   node->ts = -1;

   node->G = G;
   add_node(G, node);

   node->neighbors = create_list();

   return node;
}

void free_node(node_t* node) {
    list_free(node->neighbors, NULL);
    free(node->name);
    free(node);
}

void add_neighbor(node_t* node, node_t* neighbor) {
    list_add(node->neighbors, neighbor);
}

void update_node(node_t* node, int ts, list_t* neighbor_name) {
    item_t* item;
    char* name;
    node_t* n;

    if (ts < node->ts) return;

    list_free(node->neighbors, NULL);
    node->neighbors = create_list();

    ITER_LIST(item, neighbor_name) {
        name = item->content;
        n = get_node_by_name(node->G, name);
        if (n == NULL)
            n = create_node(node->G, name);
        list_add(node->neighbors, n);
    }
    node->ts = ts;
}

graph_t* create_graph() {
    graph_t* G = malloc(sizeof(graph_t));
    G->nodes = create_list();
    return G;
}

void free_graph(graph_t* G) {
    item_t* item;
    node_t* n;

    ITER_LIST(item, G->nodes) {
        n = item->content;
        free_node(n);
    }
    list_free(G->nodes, NULL);
    free(G);
}

void add_node(graph_t* G, node_t* n) {
    list_add(G->nodes, n);
}

node_t* get_node_by_name(graph_t* G, char* name) {
    item_t* item;
    node_t* node;

    ITER_LIST(item, G->nodes) {
        node = item->content;
        if (strcmp(name, node->name) == 0)
            return node;
    }

    return NULL;
}
