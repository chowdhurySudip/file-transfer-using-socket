#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define BUFSIZE 512
#define TRUE 1
#define LOCALHOST "127.0.0.1"

void write_file(char *filename, int client_socket);

int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 25246;
    char *filename = "new_hello_world.txt";

    setbuf(stdout, NULL);

    struct addrinfo my_server, *all_addrs, *p;
    int status;
    char port[6];
    int client_socket;

    memset(&my_server, 0, sizeof(my_server));
    my_server.ai_family = AF_INET;
    my_server.ai_socktype = SOCK_STREAM;

    sprintf(port, "%d", portno);

    if (strcmp(hostname,"localhost") == 0){
        hostname = LOCALHOST;
    }

    if((status=getaddrinfo(hostname, port, &my_server, &all_addrs)) != 0){
        fprintf(stderr, "Client: Error in getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    for(p=all_addrs; p!=NULL; p=p->ai_next){
        if((client_socket=socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            fprintf(stderr, "Client: Error in socket");
            continue;
        }

        if(connect(client_socket, p->ai_addr, p->ai_addrlen) == -1){
            close(client_socket);
            fprintf(stderr, "Client: Error in connect");
            continue;
        }

        break;
    }

    freeaddrinfo(all_addrs);

    if(p==NULL){
        fprintf(stderr, "Client: Server failed to connect");
        exit(1);
    }

    write_file(filename, client_socket);

    close(client_socket);

    return 0;
}

void write_file(char *filename, int client_socket){
    FILE *fp;
    char data[BUFSIZE];
    int rec, wr;

    fp = fopen(filename, "w");
    if(fp == NULL){
        fprintf(stderr, "Client: Error in opening file.");
        exit(1);
    }

    rec = recv(client_socket, data, BUFSIZE, 0);
    if(rec == -1){
        fprintf(stderr, "Client: Error in recv");
        exit(1);
    }

    while(rec > 0){

        wr = fwrite(data, 1, rec, fp);
        if(wr != rec){
            fprintf(stderr, "Client: Error in fwrite");
            exit(1);
        }

        rec = recv(client_socket, data, BUFSIZE, 0);
        if(rec == -1){
            fprintf(stderr, "Client: Error in recv");
            exit(1);
        }
    }

    fclose(fp);

    return;
}