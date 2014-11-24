#include <string.h>
#include <stdlib.h>
#include "graph.h"
#include "../utils/log.h"

node_t* create_node(graph_t* G, char* name) {
   node_t* node = malloc(sizeof(node_t));

   // Make a copy of the string name
   node->name = malloc(strlen(name) + 1);
   strcpy(node->name, name);

   node->id = G->nodes->len;

   node->ts = -1;

   // Put the node in the graph
   node->G = G;
   add_node(G, node);

   // Create an empty neighbor list
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

    // Discard data with older timestamp
    if (ts < node->ts) return;

    // Discard old neighbor list and create a new one
    list_free(node->neighbors, NULL);
    node->neighbors = create_list();

    ITER_LIST(item, neighbor_name) {
        name = item->content;

        n = get_node_by_name(node->G, name);

        // Create new node if the name does not exist in G
        if (n == NULL)
            n = create_node(node->G, name);

        list_add(node->neighbors, n);
    }
    // Update timestamp
    node->ts = ts;
}

graph_t* create_graph() {
    graph_t* G = malloc(sizeof(graph_t));
    G->nodes = create_list();
    G->DVs = create_list();
    return G;
}

void free_graph(graph_t* G) {
    item_t* item;
    node_t* n;

    // Free all nodes
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

node_t* get_node_by_id(graph_t* G, int id) {
    item_t* item;
    node_t* node;

    ITER_LIST(item, G->nodes) {
        node = item->content;
        if (node->id == id)
            return node;
    }

    return NULL;
}

distvector_t* create_distvector(graph_t* G, node_t* node) {
    distvector_t* d = malloc(sizeof(distvector_t));

    d->node = node;
    d->dist = malloc(G->nodes->len * sizeof(int));

    return d;
}

void calculate(graph_t* G, distvector_t* dv) {
    int n = G->nodes->len;
    int i, j, min_dist, min_id;
    int* flag = malloc(n * sizeof(int));
    node_t *node, *n2;
    item_t* item;

    for (i = 0; i < n; ++i)
        dv->dist[i] = INF;
    dv->dist[dv->node->id] = 0;
    memset(flag, 0, sizeof(int) * n);

    /* Dijkstra algorithm */
    for (i = 0; i < n - 1; ++i) {
        min_dist = INF;
        for (j = 0; j < n; ++ j)
            if (flag[j] == 0 && dv->dist[j] < min_dist) {
                min_dist = dv->dist[j];
                min_id = j;
            }

        if (min_dist == INF) break;

        flag[min_id] = 1;

        node = get_node_by_id(G, min_id);
        ITER_LIST(item, node->neighbors) {
            n2 = item->content;
            if (dv->dist[n2->id] > dv->dist[min_id] + 1)
                dv->dist[n2->id] = dv->dist[min_id] + 1;
        }
    }

    free(flag);
}

void print_dv(graph_t* G) {
    int n = G->nodes->len;
    int i;
    item_t* item;
    distvector_t* dv;

    ITER_LIST(item, G->DVs) {
        dv = item->content;

        log_msg(L_DEBUG, "%d %s\n", dv->node->id, dv->node->name);
        for (i = 0; i < n; ++i)
            log_msg(L_DEBUG, "%d ", dv->dist[i]);
        log_msg(L_DEBUG, "\n");
    }
}
