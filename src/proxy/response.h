#ifndef RESPONSE_H
#define RESPONSE_H

#define MAXLINE 8192

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

typedef struct response_line {
    char data[MAXLINE];
    struct response_line* next;
} response_line;

typedef struct response {
    int status_code;
    struct response_line *head;
    struct response_line *tail;
    int body_size;
    int body_sent;
    char *body_buf;
} response;

/**
 * Create a response object
 */
response *create_response();
/**
 * Finalize a response object
 */
void response_free(response *res);
/**
 * Create a response line of the given content
 */
response_line* create_response_line(char* buf);
/**
 * Add a response line to a response
 */
void response_add_line(response* res, response_line* line);
/**
 * Pop a response line from a response
 */
response_line *response_pop_line(response *res);

#endif
