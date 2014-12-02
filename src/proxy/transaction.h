#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "message.h"

#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define REQUEST_TIME_OUT 408
#define LENGTH_REQUIRED 411
#define REQUEST_TOO_LARGE 413
#define URI_TOO_LARGE 414
#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define SERVICE_UNAVAILABLE 503
#define VERSION_NOT_SUPPORT 505

/**
 * Transaction parsing and dealing life cycle
 */
typedef enum {
    START, PROXY, DONE, READY
} transaction_stage;

typedef enum {GET, POST, HEAD, NONE} request_method;

typedef struct transaction_node {
    /* Transaction stage */
    long long start_time;
    long long finish_time;
    transaction_stage stage;
    request_method method;
    char uri[MAXLINE];
    char path[MAXLINE];
    char filename[MAXLINE];
    char extension[MAXLINE];
    char query[MAXLINE];
    char cookie[MAXLINE];
    struct message *request_msg;
    int status_code;
    struct message *response_msg;
    struct transaction_node* next;
    char close;
    /* Indicates that this transaction is issued by proxy rather than client */
    char special;
} transaction_node;

typedef struct transaction_queue {
    transaction_node* head;
    transaction_node* tail;
} transaction_queue;

/**
 * Create a new transaction node dealing the http request
 */
transaction_node *create_transaction_node();
/**
 * Clone a new transaction from the existing one
 */
transaction_node *transaction_node_clone(transaction_node *node);
/**
 * Create a new transaction node from a request message
 */
transaction_node *create_transaction_from_msg(message *msg);
/**
 * Parse the transaction uri, extracting its path, file name,
 *file extension and query
 */
void parse_transaction_uri(transaction_node *node);
/**
 * Finalize a http transaction
 */
void transaction_node_free(transaction_node* node);
/**
 * Create a transaction queue
 */
transaction_queue *create_transaction_queue();
/**
 * Add a transaction to the end of the transaction queue
 */
void transaction_queue_add_node(transaction_queue *queue,
                                transaction_node *node);
/**
 * Add a transaction to the head of the transaction queue
 */
void transaction_queue_insert_head(transaction_queue *queue,
                                   transaction_node *node);
/**
 * Pop the first transaction from the transaction queue
 */
transaction_node *transaction_queue_pop(transaction_queue *queue);
/**
 * Finalize a transaction queue and free all its transactions
 */
void transaction_queue_free(transaction_queue *queue);
/**
 * Genereate error response for a request according to the error code
 * The node's response_msg will be NULL on error, check it after calling
 * this function.
 */
void generate_error_response(transaction_node *node);

#endif
