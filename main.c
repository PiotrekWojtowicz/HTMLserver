#include <stdio.h>
#include "webserver.h"

/*
 * Arguments should be passed as follows
 * <program_name> <port>
 */

int main(int argc, char **argv) {

    int listenfd, connfd, port;
    unsigned clientlen;
    struct sockaddr_in client_addr;

    if(argc != 2){
        fprintf(stderr, "Expected %d arguments.\n", argc);
        exit(EXIT_FAILURE);
    }

    /*
     * Set the signal to terminate
     */
    if(signal(SIGINT, my_handler) == SIG_ERR){
        perror("Could not set the signal hanlder");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    while (1){
        clientlen = sizeof(client_addr);
        connfd = Accept(listenfd, (SA *)&client_addr, &clientlen);
        Procced(connfd);
        Close(connfd);
    }
    return 0;
}
