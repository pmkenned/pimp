#include "csapp.h"

// TODO: modifying client table variables is currently NOT thread safe
// TODO: determine if realloc of client table can cause problems...
// TODO: create thread that allows user to type commands into server

struct client_conn
{
    int fd;
    struct sockaddr_in addr;
    int active;
};

int num_clients;
int client_tab_cap;
struct client_conn * client_tab;

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    int i;

    Rio_readinitb(&rio, connfd);
    while( (n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received: %d bytes\n", n);

        if(strcmp(buf, "$q\n") == 0) {
            break;
        }

        for(i=0; i < client_tab_cap; i++) {
            int fd = client_tab[i].fd;
            if(!client_tab[i].active)
                continue;
            if(fd == connfd)
                continue;
            Rio_writen(fd, buf, n);
        }
    }
}

void * thread_routine(void * vargp) {
    struct client_conn * info = ((struct client_conn*)vargp);
    int connfd = info->fd;
    struct sockaddr_in client_addr = info->addr;
    struct hostent *hp;
    char * haddrp;

    Pthread_detach(pthread_self());

    hp = Gethostbyaddr((const char *)&client_addr.sin_addr.s_addr,
                       sizeof(client_addr.sin_addr.s_addr), AF_INET);
    haddrp = inet_ntoa(client_addr.sin_addr);
    printf("server connected to %s (%s)\n", hp->h_name, haddrp);

    echo(connfd);

    printf("closing connection with %s (%s)\n", hp->h_name, haddrp);
    Close(connfd);

    num_clients--;
    info->active = 0;
    printf("num_clients: %d\n",num_clients);

    return NULL;
}

void * keyboard(void * vargp) {
    char buf[MAXLINE];
    int i;

    printf("> ");

    while(fgets(buf, MAXLINE, stdin) != NULL) {
        if(strncmp(buf, "n\n", MAXLINE) == 0) {
            printf("n: %d\n",num_clients);
        }
        else if(strncmp(buf, "q\n", MAXLINE) == 0) {
            strcpy(buf, "$q");
            for(i=0; i < client_tab_cap; i++) {
                int fd = client_tab[i].fd;
                if(client_tab[i].active) {
                    Rio_writen(fd, buf, strlen(buf));
                    close(fd);
                    client_tab[i].active = 0;
                }
            }
            exit(0);
        }
        else {
            printf("unrecognized command\n");
        }

        printf("> ");
    }

}

int main(int argc, char * argv[]) {

    int listenfd, clientlen;
    int port;

    pthread_t tid;

    num_clients = 0;
    client_tab_cap = 10;
    client_tab = malloc(client_tab_cap*sizeof(*client_tab));

    if(argc < 2) {
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    Pthread_create(&tid, NULL, keyboard, NULL);

    listenfd = Open_listenfd(port);

    while(1) {

        struct client_conn * info;

        int i;

        /* first, look for an inactive client connection */
        int found = 0;
        for(i=0; i < client_tab_cap; i++) {
            if(!client_tab[i].active) {
                info = &(client_tab[i]);
                found = 1;
                break;
            }
        }

        /* if no inactive found, need to allocate more */
        if(!found) {
            client_tab_cap *= 2;
            client_tab = realloc(client_tab, client_tab_cap*sizeof(*client_tab));

            info = &(client_tab[num_clients]);
        }

        clientlen = sizeof(info->addr);
        info->fd = Accept(listenfd, (struct sockaddr *)&(info->addr), &clientlen);
        info->active = 1;

        num_clients++;
        printf("num_clients: %d\n",num_clients);

        Pthread_create(&tid, NULL, thread_routine, info);

    }

    return 0;
}
