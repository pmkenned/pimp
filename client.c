#include <stdio.h>

#include "csapp.h"

// TODO: need a way to interrupt the Fgets call if server closes connection

int active;

void * keyboard(void * vargp) {
    char buf[MAXLINE];
    rio_t rio;

    int clientfd = *((int*) vargp);

    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL) {
        if(active)
            Rio_writen(clientfd, buf, strlen(buf));
        else
            break;
    }

    if(active) {
        strcpy(buf, "$q\n");
        Rio_writen(clientfd, buf, strlen(buf));
    }

    return NULL;
}

int main(int argc, char * argv[]) {

    int clientfd;
    char *host, buf[MAXLINE];
    int port;
    rio_t rio;
    pthread_t tid;

    if(argc < 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = atoi(argv[2]);

    clientfd = Open_clientfd(host, port);

    active = 1;

    Pthread_create(&tid, NULL, keyboard, &clientfd);

    Rio_readinitb(&rio, clientfd);
    while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {

        if(strcmp(buf, "$q") == 0) {
            printf("received $q command from server\n");
            active = 0;
        }
        else {
            Fputs(buf, stdout);
        }
    }

    Close(clientfd);

    /* note: if the server closes the connection, the keyboard thread
       will be in Fgets and will try to write to a closed connection... */

    return 0;
}
