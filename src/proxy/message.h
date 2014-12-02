#ifndef MESSAGE_H
#define MESSAGE_H

#define MAXLINE 8192
#define MAX_PATH_LENGTH 4096
#define MAX_BODY_SIZE 1048576

/**
 * Message parsing life cycle
 */
typedef enum {
    MSG_LINE, MSG_BODY, MSG_ABORT, MSG_DONE
} message_stage;

typedef struct message_line {
    char data[MAXLINE];
    struct message_line *next;
} message_line;

typedef struct message {
    message_stage stage;
    struct message_line *head;
    struct message_line *tail;
    int body_size;
    int body_read;
    int body_sent;
    char *body_buf;
} message;

/**
 * Create an empty message structure
 */
message *create_http_message();
/**
 * Clone a new message from the existing one
 */
message *message_clone(message *msg);
/**
 * Finalize a message structure
 */
void message_free(message *msg);
/**
 * Create a message line of the given string
 */
message_line *create_message_line(char* buf);
/**
 * Add a message line to a message
 */
void message_add_line(message *msg, message_line *line);
/**
 * Pop a message line from a message
 * Often used after the line had been successfully sent
 */
message_line *message_pop_line(message *msg);

#endif
