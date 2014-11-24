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
 * Print informations of network graph
 */
void print_graph();

/**
 * Return the ip of nearest video server
 *
 * @param  domain  The domain name to be resolved
 * @param  from_ip The ip of the client
 * @return         ip of the nearest video server. NULL if given client ip does
 *                 not exist in the network
 */
char* nearest_server(char* domain, char* from_ip);

#endif
