#include <stdio.h>

#include "csapp.h"

// TODO: need a way to interrupt the Fgets call if server closes connection

int active;

void * keyboard(void * vargp) {
    char buf[MAXLINE];
    rio_t rio;

    fprintf(stderr,"fprintf 7\n");

    int clientfd = *((int*) vargp);

    fprintf(stderr,"fprintf 8\n");

    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL) {
        fprintf(stderr,"fprintf 9\n");
        if(active)
            Rio_writen(clientfd, buf, strlen(buf));
        else
            break;
    }

    if(active) {
        fprintf(stderr,"fprintf 10\n");
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

    fprintf(stderr,"fprintf 1\n");

    if(argc < 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = atoi(argv[2]);

    fprintf(stderr,"fprintf 2\n");

    clientfd = Open_clientfd(host, port);

    fprintf(stderr,"fprintf 3\n");

    active = 1;

    Pthread_create(&tid, NULL, keyboard, &clientfd);

    fprintf(stderr,"fprintf 4\n");

    Rio_readinitb(&rio, clientfd);
    while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {

        fprintf(stderr,"fprintf 5\n");

        if(strcmp(buf, "$q") == 0) {
            printf("received $q command from server\n");
            active = 0;
        }
        else {
            fprintf(stderr,"fprintf 6\n");
            Fputs(buf, stdout);
        }
    }

    Close(clientfd);

    /* note: if the server closes the connection, the keyboard thread
       will be in Fgets and will try to write to a closed connection... */

    return 0;
}
