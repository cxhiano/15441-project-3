#ifndef REQUEST_H
#define REQUEST_H

#include "response.h"

#define MAX_PATH_LENGTH 4096
#define MAXLINE 8192
#define MAX_HEADER_NUM 128
#define MAX_BODY_SIZE 1048576

typedef enum {GET, POST, HEAD, NONE} request_method;

typedef enum {
    START, HEADER, BODY, ABORT, DONE, PROXY, READY
} request_stage;

typedef struct request_headers {
    char accept[MAXLINE];
    char accept_charset[MAXLINE];
    char accept_encoding[MAXLINE];
    char accept_language[MAXLINE];
    char content_type[MAXLINE];
    char cookie[MAXLINE];
    char host[MAXLINE];
    char referer[MAXLINE];
    char user_agent[MAXLINE];
} request_headers;

typedef struct request_node {
    /* Request parsing stage */
    request_stage stage;
    /* Request info */
    request_method method;
    char uri[MAXLINE];
    char query[MAXLINE];
    struct request_headers headers;
    int header_num;
    int body_size;
    int body_read;
    int body_write;
    char* body_buf;
    struct response *res;
    char close;
    struct request_node* next;
} request_node;

typedef struct request_queue {
    request_node* head;
    request_node* tail;
} request_queue;

/**
 * Create a new request node representing the http request
 */
request_node* create_request_node();
/**
 * Finalize a http request
 */
void request_node_free(request_node* node);
/**
 * Create a request queue
 */
request_queue* create_request_queue();
/**
 * Add a request to the end of the request queue
 */
void request_queue_add_node(request_queue* queue, request_node* node);
/**
 * Pop the first request from the request queue
 */
request_node* request_queue_pop(request_queue* queue);
/**
 * Finalize a request queue and free all its requests
 */
void request_queue_free(request_queue* queue);
/**
 * Parse the request line of the request
 */
void parse_request_line(request_node* node, char* line_buf);
/**
 * Parse the headers of the request
 */
void parse_request_header(request_node* node, char* line_buf);

/**
 * Genereate error response for a request according to the error code
 */
void generate_error_response(request_node *node);

#endif
