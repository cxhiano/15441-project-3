#include <stdlib.h>
#include "config.h"
#include "server.h"
#include "../utils/log.h"

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
    dns_ip = argv[5];
    dns_port = atoi(argv[6]);
    if (argc == 8)
        www_ip = argv[7];

    serve(listen_port);

    return 0;

}