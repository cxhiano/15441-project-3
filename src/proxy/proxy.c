#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "proxy.h"
#include "log.h"
#include "../utils/net.h"

char *www_ip = NULL;
char *fake_ip = NULL;
double alpha = 0;
video_bitrates *video = NULL;

long long now()
{
    struct timeval now;
    long long microsecond;

    gettimeofday(&now, NULL);
    microsecond = now.tv_sec;
    microsecond = microsecond * 1000000 + now.tv_usec;

    return microsecond;
}

video_bitrates *create_video()
{
    video_bitrates *video;

    video = (video_bitrates *)malloc(sizeof(video_bitrates));
    if (video == NULL)
        return NULL;
    video->filename[0] = '\0';
    video->throughput = 0;
    video->bitrate_num = 0;

    return video;
}

int video_add_bitrate(video_bitrates *video, int bitrate)
{
    if (video->bitrate_num < BITRATE_NUM_MAX) {
        video->bitrates[video->bitrate_num] = bitrate;
        video->bitrate_num++;
        return 0;
    } else {
        return -1;
    }
}

void proxy_initialize(char *new_www_ip, char *new_fake_ip, double new_alpha)
{
    if (www_ip == NULL)
        www_ip = (char *)malloc(strlen(new_www_ip)+1);
    if (www_ip == NULL) {
        fprintf(stderr, "No memory to record www ip");
        exit(EXIT_FAILURE);
    }
    strcpy(www_ip, new_www_ip);
    if (fake_ip == NULL)
        fake_ip = (char *)malloc(strlen(new_fake_ip)+1);
    if (fake_ip == NULL) {
        fprintf(stderr, "No memory to record fake ip");
        exit(EXIT_FAILURE);
    }
    strcpy(fake_ip, new_fake_ip);
    alpha = new_alpha;
}

void handle_proxy_session(proxy_session *session)
{
    transaction_node *node;
    transaction_node *temp;
    transaction_node *special_node;
    message_line *line;
    char *cookie;
    char modified_line[MAXLINE];
    char filename[MAXLINE];
    int bitrate, i;

    temp = NULL;
    node = session->queue->head;
    while (node) {
        if (node->special) {
            break;
        }
        if (node->stage == START) {
            fprintf(stderr, "%s\n", node->filename);
            if (session->server_conn->conn_fd == -1) {
                if (connect_to_server(session->server_conn) == -1) {
                    return;
                }
            }
            if (strcmp(node->extension, "f4m") == 0 && !node->special &&
                    node->method == GET) {
                special_node = transaction_node_clone(node);
                if (special_node == NULL) {
                    return;
                }
                special_node->special = 1;
                if (temp == NULL) {
                    transaction_queue_insert_head(session->queue,
                                                  special_node);
                    temp = special_node;
                } else {
                    temp->next = special_node;
                    special_node->next = node;
                }
                special_node->stage = PROXY;
                line = node->request_msg->head;
                sprintf(modified_line, "%s%s_nolist.%s", node->path,
                        node->filename, node->extension);
                if (strlen(node->query) > 0) {
                    sprintf(modified_line, "%s?%s", modified_line, node->query);
                }
                sprintf(line->data, "GET %s HTTP/1.1\r\n", modified_line);
                node->stage = PROXY;
                break;
            } else if (strstr(node->filename, "-Frag") != NULL &&
                       node->method == GET) {
                if (video == NULL) {
                    fprintf(stderr, "No video info\n");
                    cookie = strstr(node->cookie, "filename=");
                    if (cookie != NULL) {
                        filename[0] = '\0';
                        sscanf(cookie, "filename=%s", filename);
                        if (strlen(filename) > 0) {
                            special_node = transaction_node_clone(node);
                            if (special_node == NULL) {
                                return;
                            }
                            special_node->special = 1;
                            line = special_node->request_msg->head;
                            sprintf(modified_line, "%s%s.f4m", node->path,
                                    filename);
                            sprintf(line->data, "GET %s HTTP/1.1\r\n",
                                    modified_line);
                            special_node->stage = PROXY;
                            transaction_queue_insert_head(session->queue,
                                                          special_node);
                            return;
                        }
                    }
                    node->start_time = now();
                    node->stage = PROXY;
                } else {
                    sscanf(node->filename, "%dSeg%s", &bitrate, filename);
                    fprintf(stderr, "%d\t%s\n", bitrate, filename);
                    bitrate = 0;
                    for (i = 0; i < video->bitrate_num; i++) {
                        if (1.5 * video->bitrates[i] <=
								video->throughput &&
                                video->bitrates[i] > bitrate) {
                            bitrate = video->bitrates[i];
                        }
                    }
                    fprintf(stderr, "%d\t%f\n", bitrate, video->throughput);
                    if (bitrate != 0) {
                        line = node->request_msg->head;
                        sprintf(modified_line, "%s%dSeg%s", node->path, bitrate,
                                filename);
                        sprintf(node->filename, "%dSeg%s", bitrate, filename);
                        sprintf(line->data, "GET %s HTTP/1.1\r\n",
                                modified_line);
                    } else {
                        line = node->request_msg->head;
                        sprintf(modified_line, "%s%dSeg%s", node->path, 10,
                                filename);
                        sprintf(node->filename, "%dSeg%s", 10, filename);
                        sprintf(line->data, "GET %s HTTP/1.1\r\n",
                                modified_line);
					}
                    node->start_time = now();
                    node->stage = PROXY;
                }
            } else {
                node->stage = PROXY;
            }
        }
        if (node->close)
            break;
        temp = node;
        node = node->next;
    }
}

int connect_to_server(connection *conn)
{
    sockaddr_in_t serveraddr, clientaddr;

    if ((conn->conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Fail to create socket\n");
        conn->conn_fd = -1;
        return -1;
    }
    clientaddr = make_sockaddr_in(fake_ip, 0);
    if (bind(conn->conn_fd, (struct sockaddr *)&clientaddr,
             sizeof(clientaddr))) {
        fprintf(stderr, "Fail to bind to fake ip\n");
        close(conn->conn_fd);
        conn->conn_fd = -1;
        return -1;
    }
    serveraddr = make_sockaddr_in(www_ip, SERVER_PORT);
    if (connect(conn->conn_fd, (struct sockaddr *) &serveraddr,
                sizeof(serveraddr)) < 0) {
        fprintf(stderr, "Fail to bind to www ip\n");
        close(conn->conn_fd);
        conn->conn_fd = -1;
        return -1;
    }
    return 0;
}

void parse_bitrates(transaction_node *node)
{
    char *bitrate_tag = NULL;
    int bitrate;

    free(video);
    video = create_video();
    if (video == NULL)
        return;
    if (node->response_msg->body_size <= 0)
        return;
    node->response_msg->body_buf[node->response_msg->body_size-1] = '\0';
    bitrate_tag = node->response_msg->body_buf;
    while ((bitrate_tag = strstr(bitrate_tag, "bitrate=")) != NULL) {
        sscanf(bitrate_tag, "bitrate=\"%d\"", &bitrate);
        video_add_bitrate(video, bitrate);
        fprintf(stderr, "%d ", bitrate);
        bitrate_tag++;
    }
}

void update_throughput(transaction_node *node)
{
    long long log_now;
    double duration;
    double throughput;
    int bitrate;
    char buf[MAXLINE];

    log_now = now() / 1000000;
    duration = (node->finish_time - node->start_time) / 1000000.0;
    throughput = node->response_msg->body_size * 8 / duration / 1000.0;
    video->throughput = alpha * throughput +
                                 (1 - alpha) * video->throughput;
    sscanf(node->filename, "%dSeq%s", &bitrate, buf);
    log_log("%lld %f %f %f %d %s %s\n", log_now, duration, throughput,
            video->throughput, bitrate, www_ip, node->filename);
}
