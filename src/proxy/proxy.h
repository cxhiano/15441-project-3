#ifndef PROXY_H
#define PROXY_H

#include "server.h"
#include "transaction.h"

#define BITRATE_NUM_MAX 128
#define SERVER_PORT 8080
#define SERVER_PORT_STR "8080"

typedef struct video_bitrates {
    char filename[MAXLINE];
    double throughput;
    int bitrate_num;
    int bitrates[BITRATE_NUM_MAX];
} video_bitrates;

/**
 * Return time since epoch in microseconds
 */
long long now();
/**
 * Create a video structure represents the available bitrates of a video
 */
video_bitrates *create_video();
/**
 * Add an available bitrate to a video
 * Return 0 on success, -1 on error (too many bitrates)
 */
int video_add_bitrate(video_bitrates *video, int bitrate);
/**
 * Initialize the proxy with the given www ip, fake ip and alpha.
 */
void proxy_initialize(char *new_www_ip, char *new_fake_ip, double new_alpha);
/**
 * Handle a connection's request by alter its uri and forward to the server
 */
void handle_proxy_session(proxy_session *session);
/**
 * Establish a connection to the server
 * Return 0 on success, -1 on error
 */
int connect_to_server(connection *conn);
/**
 * Parse the f4m xml and get available bitrates
 */
void parse_bitrates(transaction_node *node);
/**
 * Log the current request and update throughput
 */
void update_throughput(transaction_node *node);

#endif
