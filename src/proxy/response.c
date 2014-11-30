#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "response.h"

response *create_response()
{
    response *res;

    res = (response *)malloc(sizeof(response));
    if (res == NULL)
        return NULL;
    res->status_code = 0;
    res->head = NULL;
    res->tail = NULL;
    res->body_buf = NULL;
    res->body_size = 0;
    res->body_sent = 0;

    return res;
}

void response_free(response *res)
{
    response_line* line;
    response_line* temp;

    if (res->body_buf != NULL)
        free(res->body_buf);
    line = res->head;
    while (line) {
        temp = line->next;
        free(line);
        line = temp;
    }
    free(res);
}

response_line* create_response_line(char* buf)
{
    response_line* line;

    line = (response_line *)malloc(sizeof(response_line));
    if (line == NULL) {
        fprintf(stderr, "No memory to create response line\n");
        return NULL;
    }
    strcpy(line->data, buf);
    line->next = NULL;

    return line;
}

void response_add_line(response* res, response_line* line)
{
    if (res->head == NULL) {
        res->head = line;
        res->tail = line;
    } else {
        res->tail->next = line;
        res->tail = line;
    }
}

response_line *response_pop_line(response *res)
{
    response_line* line;

    if (res->head == NULL)
        return NULL;
    line = res->head;
    if (line->next == NULL) {
        res->head = NULL;
        res->tail = NULL;
    } else {
        res->head = line->next;
    }
    return line;
}

