#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transaction.h"

transaction_node *create_transaction_node()
{
    transaction_node *node;

    node = (transaction_node *)malloc(sizeof(transaction_node));
    if (node == NULL)
        return NULL;
    node->start_time = 0;
    node->finish_time = 0;
    node->stage = START;
    node->method = NONE;
    node->uri[0] = '\0';
    node->path[0] = '\0';
    node->filename[0] = '\0';
    node->extension[0] = '\0';
	node->query[0] = '\0';
    node->cookie[0] = '\0';
    node->request_msg = NULL;
    node->response_msg = NULL;
    node->status_code = 0;
    node->next = NULL;
    node->close = 0;
    node->special = 0;

    return node;
}

transaction_node *transaction_node_clone(transaction_node *node)
{
	transaction_node *new_node;

	if (node == NULL)
		return NULL;
	new_node = create_transaction_node();
	if (new_node == NULL)
		return NULL;
	new_node->start_time = node->start_time;
	new_node->finish_time = node->finish_time;
	new_node->stage = node->stage;
	new_node->method = node->method;
	strcpy(new_node->uri, node->uri);
	strcpy(new_node->path, node->path);
	strcpy(new_node->filename, node->filename);
	strcpy(new_node->extension, node->extension);
	strcpy(new_node->query, node->query);
	strcpy(new_node->cookie, node->cookie);
	if (node->request_msg != NULL) {
		if ((new_node->request_msg = message_clone(node->request_msg)) == NULL){
			transaction_node_free(new_node);
			return NULL;
		}
	}
	if (node->response_msg != NULL) {
		if ((new_node->response_msg = message_clone(node->response_msg))==NULL){
			transaction_node_free(new_node);
			return NULL;
		}
	}
	new_node->status_code = node->status_code;
	new_node->next = NULL;
	new_node->close = node->close;
	new_node->special = node->special;

	return new_node;
}

transaction_node *create_transaction_from_msg(message *msg)
{
	transaction_node *node = NULL;
	message_line *line = NULL;
	int n;
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE], token[MAXLINE];

	if ((node = create_transaction_node()) == NULL) {
		return NULL;
	}
	node->request_msg = msg;
	if (msg->stage != MSG_DONE) {
		node->status_code = BAD_REQUEST;
		node->close = 1;
		return node;
	}
	/* Parse request line */
	line = msg->head;
	n = sscanf(line->data, "%s %s %s", method, uri, version);
	if (n != 3) {
		node->method = NONE;
		node->status_code = BAD_REQUEST;
		node->close = 1;
		return node;
	}
	if (strstr(method, "GET") == method)
        node->method = GET;
    else if (strstr(method, "POST") == method)
        node->method = POST;
    else if (strstr(method, "HEAD") == method)
        node->method = HEAD;
    else {
        node->method = NONE;
        node->status_code = NOT_IMPLEMENTED;
        node->close = 1;
        return node;
    }
    if (node->method == POST && msg->body_size == -1) {
    	node->status_code = LENGTH_REQUIRED;
    	node->close = 1;
    	return node;
    }
    if (strstr(version, "HTTP/1.1") != version) {
    	node->status_code = VERSION_NOT_SUPPORT;
    	node->close = 1;
    	return node;
    }
    strcpy(node->uri, uri);
    parse_transaction_uri(node);
    line = line->next;
	while (line) {
		if (strstr(line->data, "Connection:") == line->data) {
	        sscanf(line->data, "Connection: %s", token);
	        if (strcmp(token, "close") == 0 || strcmp(token, "Close") == 0) {
	        	fprintf(stderr, "Client wants to close\n");
	        }
	    } else if (strstr(line->data, "Cookie:") == line->data) {
	        sscanf(line->data, "Cookie: %[^\r^\n]", node->cookie);
	    }
		line = line->next;
	}
	return node;
}

void parse_transaction_uri(transaction_node *node)
{
	char* path;
	char* filename;
	char* extension;
    char* query;
    char* rear_slash;
    char* rear_dot;
    char* colon_slash;

    if ((colon_slash = strstr(node->uri, "://")) != NULL)
        path = strchr(colon_slash+3, '/');
    else
        path = strchr(node->uri, '/');
    if (path == NULL) {
    	strcpy(node->uri, "/index.html");
    	strcpy(node->path, "/");
    	strcpy(node->filename, "index");
    	strcpy(node->extension, "html");
    	return;
    }
    query = strchr(path, '?');
    if (query)
    {
    	strcpy(node->query, query+1);
        *query = '\0';
    }
    if (strlen(path) == 1) {
    	strcpy(node->uri, "/index.html");
    	strcpy(node->path, "/");
    	strcpy(node->filename, "index");
    	strcpy(node->extension, "html");
    	return;
    }
    rear_slash = strrchr(path, '/');
    if (rear_slash == NULL) {
    	strcpy(node->uri, "/index.html");
    	strcpy(node->path, "/");
    	strcpy(node->filename, "index");
    	strcpy(node->extension, "html");
    	return;
    }
    *rear_slash = '\0';
    filename = rear_slash + 1;
    rear_dot = strrchr(filename, '.');
    if (rear_dot == NULL) {
    	strcpy(node->path, path);
    	strcat(node->path, "/");
    	strcpy(node->filename, filename);
	    sprintf(node->uri, "%s%s", path, filename);
    	return;
    }
    *rear_dot = '\0';
    extension = rear_dot + 1;
    strcpy(node->path, path);
    strcat(node->path, "/");
    strcpy(node->filename, filename);
    strcpy(node->extension, extension);
    sprintf(node->uri, "%s%s.%s", path, filename, extension);
}

void transaction_node_free(transaction_node* node)
{
	if (node->request_msg != NULL) {
		message_free(node->request_msg);
	}
	if (node->response_msg != NULL) {
		message_free(node->response_msg);
	}
	free(node);
}

transaction_queue* create_transaction_queue()
{
    transaction_queue* queue;

    queue = (transaction_queue *)malloc(sizeof(transaction_queue));
    if (queue == NULL)
        return NULL;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void transaction_queue_add_node(transaction_queue* queue,
	transaction_node* node)
{
    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

void transaction_queue_insert_head(transaction_queue *queue,
    transaction_node *node)
{
	if (queue->head == NULL) {
		queue->head = node;
		queue->tail = node;
	} else {
		node->next = queue->head;
		queue->head = node;
	}
}

transaction_node* transaction_queue_pop(transaction_queue* queue)
{
    transaction_node* node;

    if (queue->head == NULL) {
        return NULL;
    }
    node = queue->head;
    if (node->next == NULL) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = node->next;
    }
    return node;
}

void transaction_queue_free(transaction_queue* queue)
{
    transaction_node* node;
    transaction_node* temp;

    node = queue->head;
    while (node) {
        temp = node->next;
        transaction_node_free(node);
        node = temp;
    }
    free(queue);
}

void generate_error_response(transaction_node *node)
{
    message *msg = NULL;
    char body[MAXLINE], status_line[MAXLINE], content_len[MAXLINE];
    message_line* temp_line;

    if (node->response_msg != NULL) {
    	message_free(node->response_msg);
    	node->response_msg = NULL;
    }
    if ((msg = create_http_message()) == NULL) {
    	return;
    }
    node->response_msg = msg;
    sprintf(body, "<html><title>Server Error</title>");
    if (node->status_code == BAD_REQUEST) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Bad Request");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Bad request, can't parse method or uri or version");
    } else if (node->status_code == NOT_FOUND) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Not Found");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Request content not found");
    } else if (node->status_code == REQUEST_TIME_OUT) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Request Time-out");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Request time out");
    } else if (node->status_code == LENGTH_REQUIRED) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Length Required");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Content-Length required for POST");
    } else if (node->status_code == REQUEST_TOO_LARGE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Request Entity Too Large");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Request too large or request line too long");
    } else if (node->status_code == URI_TOO_LARGE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Request-URI too large");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Request uri larger than server can handle");
    } else if (node->status_code == INTERNAL_SERVER_ERROR) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Internal Server Error");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Some bugs have crawled into the server");
    } else if (node->status_code == NOT_IMPLEMENTED) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Not Implemented");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Method not implemented");
    } else if (node->status_code == SERVICE_UNAVAILABLE) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Service unavailable");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Server under maintenance or too many connections");
    } else if (node->status_code == VERSION_NOT_SUPPORT) {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "HTTP Version not supported");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "HTTP version not support by server");
    } else {
        sprintf(status_line, "HTTP/1.1 %d %s\r\n",
                node->status_code, "Unknown error");
        sprintf(body, "%s%d: %s\r\n", body, node->status_code,
                "Unknown error");
    }
    sprintf(content_len, "Content-Length: %d\r\n",
            (int)strlen(body));

    if ((temp_line = create_message_line(status_line)) == NULL) {
        node->close = 1;
        return;
    }
    message_add_line(msg, temp_line);
    if ((temp_line = create_message_line(content_len)) == NULL) {
        node->close = 1;
        return;
    }
    message_add_line(msg, temp_line);
    if (node->close) {
	    if ((temp_line = create_message_line("Connection: close\r\n")) == NULL) {
	        node->close = 1;
	        return;
	    }
	    message_add_line(msg, temp_line);
    } else {
    	if ((temp_line = create_message_line("Connection: keep-alive\r\n")) == NULL) {
	        node->close = 1;
	        return;
	    }
	    message_add_line(msg, temp_line);
    }
    if ((temp_line = create_message_line("Content-Type: text/html\r\n")) == NULL) {
        node->close = 1;
        return;
    }
    message_add_line(msg, temp_line);
    if ((temp_line = create_message_line("\r\n")) == NULL) {
    	node->close = 1;
    	return;
    }
    message_add_line(msg, temp_line);

    msg->body_size = strlen(body);
    msg->body_buf = (char *)malloc(msg->body_size);
    if (msg->body_buf == NULL) {
        fprintf(stderr, "No memory to allocate response body\n");
        node->close = 1;
        msg->body_size = -1;
        return;
    }
    memcpy(msg->body_buf, body, msg->body_size);
}
