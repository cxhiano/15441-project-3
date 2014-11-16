#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "io.h"
#include "log.h"
#include "request.h"
#include "utils/list.h"

/** @brief Create and config a socket on given port. */
static int setup_server_socket(unsigned short port) {
    static int yes = 1; //For setsockopt
    int listen_fd;
    static struct sockaddr_in server_addr;

    if ((listen_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        log_error("Failed creating socket.");
        return -1;
    }

    //Enable duplicate address and port binding
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        log_error("setsockopt failed.");
        return -1;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        close(listen_fd);
        log_error("Failed binding socket.");
        return -1;
    }

    if (listen(listen_fd, DEFAULT_BACKLOG)) {
        close(listen_fd);
        log_error("Failed listening on socket.");
        return -1;
    }

    return listen_fd;
}

void serve(unsigned short port) {
    int listen_fd, client_fd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    list_t* request_list;
    item_t* item;
    request_t* req;

    if ((listen_fd = setup_server_socket(port)) == -1) return;

    //initialize fd lists
    init_select_context();
    add_read_fd(listen_fd);

    request_list = create_list();

    /*===============Start accepting requests================*/
    client_addr_len = sizeof(client_addr);
    while (1) {
        if (io_select() == -1) {
            log_error("select error");
            continue;
        }

        //New request!
        if (test_read_fd(listen_fd)) {
            if ((client_fd = accept(listen_fd, (struct sockaddr *)&client_addr,
                                    (socklen_t *)&client_addr_len)) == -1) {
                log_error("Error accepting connection.");
                continue;
            }
            //Add socket to fd list
            add_read_fd(client_fd);
            add_write_fd(client_fd);
        }

        ITER_LIST(item, request_list) {
            req = item->content;

        }
    }

}
