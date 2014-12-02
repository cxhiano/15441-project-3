#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

message *create_http_message()
{
    message *msg;

    msg = (message *)malloc(sizeof(message));
    if (msg == NULL)
        return NULL;
    msg->stage = MSG_LINE;
    msg->head = NULL;
    msg->tail = NULL;
    msg->body_buf = NULL;
    msg->body_size = -1;
    msg->body_sent = 0;
    msg->body_read = 0;

    return msg;
}

message *message_clone(message *msg)
{
    message *new_msg;
    message_line *new_line;
    message_line *line;

    if (msg == NULL)
        return NULL;
    new_msg = create_http_message();
    if (new_msg == NULL)
        return NULL;
    line = msg->head;
    while (line) {
        new_line = create_message_line(line->data);
        if (new_line == NULL) {
            message_free(new_msg);
            return NULL;
        }
        message_add_line(new_msg, new_line);
        line = line->next;
    }
    if (msg->body_size > 0 && msg->body_buf != NULL) {
        new_msg->body_buf = (char *)malloc(msg->body_size);
        if (new_msg->body_buf == NULL) {
            message_free(new_msg);
            return NULL;
        }
        memcpy(new_msg->body_buf, msg->body_buf, msg->body_size);
    }
    new_msg->stage = msg->stage;
    new_msg->body_size = msg->body_size;
    new_msg->body_read = msg->body_read;
    new_msg->body_sent = msg->body_sent;

    return new_msg;
}

void message_free(message *msg)
{
    message_line* line;
    message_line* temp;

    if (msg->body_buf != NULL)
        free(msg->body_buf);
    line = msg->head;
    while (line) {
        temp = line->next;
        free(line);
        line = temp;
    }
    free(msg);
}

message_line *create_message_line(char* buf)
{
    message_line* line;

    line = (message_line *)malloc(sizeof(message_line));
    if (line == NULL) {
        fprintf(stderr, "No memory to create response line\n");
        return NULL;
    }
    strcpy(line->data, buf);
    line->next = NULL;

    return line;
}

void message_add_line(message *msg, message_line *line)
{
    if (msg->head == NULL) {
        msg->head = line;
        msg->tail = line;
    } else {
        msg->tail->next = line;
        msg->tail = line;
    }
}

message_line *message_pop_line(message *msg)
{
    message_line* line;

    if (msg->head == NULL)
        return NULL;
    line = msg->head;
    if (line->next == NULL) {
        msg->head = NULL;
        msg->tail = NULL;
    } else {
        msg->head = line->next;
    }
    return line;
}
