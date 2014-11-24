#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "../utils/list.h"

typedef struct node_s node_t;
typedef struct distvector_s distvector_t;
typedef struct graph_s graph_t;

struct node_s {
    char* name;     //<! The name associated with the node(must be unique)
    int id;         //<! The id of the node.
    graph_t* G;     //<! The graph this node belongs to
    int ts;         //<! The timestamp of the data associated with this node
    list_t* neighbors;  //<! A list of neighbors
};

/**
 * Create a new node in graph G.
 *
 * @param  G    The graph where the new node belongs
 * @param  name The name of the node
 * @return      A new node
 */
node_t* create_node(graph_t* G, char* name);

/**
 * Free resource used by a node.
 * @param node
 */
void free_node(node_t* node);

/**
 * Add a neighbor to a node
 */
void add_neighbor(node_t* node, node_t* neighbor);

/**
 * Update neighbors list of node. If the timestamp of the new neighbor list is
 * greater than the timestamp of the node, the neigbor list of the node will be
 * updated. New node will be created when the node with a name from the neigbor
 * list does not exist in the graph.
 *
 * @param node
 * @param ts            The timestamp of provided neighbor list.
 * @param neighbor_name A list of name of neighbors
 */
void update_node(node_t* node, int ts, list_t* neighbor_name);

struct graph_s {
    list_t* nodes; //<! A list of all nodes in the graph
    list_t* DVs;   //<! A list of several distance vectors
};

/**
 * Create a new empty graph
 */
graph_t* create_graph();

/**
 * Release resource used by the graph. All nodes in the graph will be free.
 */
void free_graph(graph_t* G);

/**
 * Add a node to graph G
 */
void add_node(graph_t* G, node_t* n);

/**
 * Return a pointer to the node with given name. Return NULL if such a node
 * cannot be found
 */
node_t* get_node_by_name(graph_t* G, char* name);

/**
 * Return a pointer to the node with given id. Return NULL if such node cannot
 * be found
 */
node_t* get_node_by_id(graph_t* G, int id);

/*
 * The limit of distance. The distance to nodes that can not be reached will be
 * set to INF.
 */
#define INF 1000000000

struct distvector_s {
    node_t* node;
    int* dist;
};

/**
 * Create a distance vector
 * @param  G    The graph corresponding to the distance vector
 * @param  node The origin of the distance vector
 */
distvector_t* create_distvector(graph_t* G, node_t* node);

/**
 * Run shortest path algorithm to calculate the distance vector
 */
void calculate(graph_t* G, distvector_t* dv);

/**
 * Print out all distance vector of given graph
 */
void print_dv(graph_t* G);

#endif
