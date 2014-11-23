#ifndef __STRATEGY_H__
#define __STRATEGY_H__

/**
 * Return ip for a domain name in round robin way
 */
char* round_robin(char* domain);

/**
 * Initialize network graph using given lsa file
 * @return          0 on success. Otherwise -1
 */
int lsa_init(char* lsa_file);

/**
 * Print net work graph
 */
void print_graph();

#endif
