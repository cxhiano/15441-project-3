#include <stdlib.h>
#include "config.h"
#include "server.h"
#include "log.h"

void usage() {
    printf("...\n");

}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        usage();
        exit(0);
    }

    log_path = argv[1];
    alpha = atof(argv[2]);
    listen_port = atoi(argv[3]);
    fake_ip = argv[4];
    dns_port = atoi(argv[5]);
    if (argc == 7)
        www_ip = argv[6];

    serve(listen_port);

    return 0;

}