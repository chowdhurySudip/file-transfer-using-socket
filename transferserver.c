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

void send_file(FILE *f, int client_socket);

int main(int argc, char **argv)
{
    int option_char;
    int portno = 25246;
    char *filename = "hello_world.txt";

    setbuf(stdout, NULL);

    struct addrinfo my_server, *all_addrs, *p;
    int status;
    char *host = LOCALHOST;
    char port[6];
    int server_sock, client_sock;
    int yes = 1;

    FILE *fp;

    memset(&my_server, 0, sizeof(my_server));
    my_server.ai_family = AF_INET;
    my_server.ai_socktype = SOCK_STREAM;

    sprintf(port, "%d", portno);

    if((status=getaddrinfo(host, port, &my_server, &all_addrs)) != 0){
        fprintf(stderr, "Server: Error in getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
  
    for(p=all_addrs; p!=NULL; p=p->ai_next){
        if((server_sock=socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            fprintf(stderr, "Server: Error in socket");
            continue;
        }

        if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            fprintf(stderr, "Server: Error in setsockopt");
            exit(1);
        }

        if(bind(server_sock, p->ai_addr, p->ai_addrlen) == -1){
            close(server_sock);
            fprintf(stderr, "Server: Error in bind");
            continue;
        }

        break;
    }

    freeaddrinfo(all_addrs);

    if(p==NULL){
        fprintf(stderr, "Server: Server failed to bind");
        exit(1);
    }

    if(listen(server_sock, 5) == -1){
        fprintf(stderr, "Server: Failed in listen");
        exit(1);
    }

    while (TRUE){
        client_sock = accept(server_sock, NULL, NULL);
        if(client_sock==-1){
            fprintf(stderr, "Server: Failed in accept");
            exit(1);
        }

        fp = fopen(filename, "r");
        if(fp == NULL){
            fprintf(stderr, "Server: Error in opening file.");
            exit(1);
        }

        send_file(fp, client_sock);

        fclose(fp);

        close(client_sock);
    }

    close(server_sock);

    return 0;
}

void send_file(FILE *f, int client_socket){
    char data[BUFSIZE];
    size_t byte_read, byte_sent, byte_rem;
    int sen;
    // memset(&data, 0, sizeof(data));

    while(!(feof(f))){
        byte_read = fread(data, 1, BUFSIZE, f);
        if(byte_read == -1){
            fprintf(stderr, "Server: Error in fread");
            exit(1);
        }

        byte_sent = 0;
        byte_rem = byte_read;
        while(byte_sent < byte_read){
            sen = send(client_socket, data+byte_sent, byte_rem, 0);
            if(sen == -1){
                fprintf(stderr, "Server: Error in send");
                exit(1);
            }
            byte_sent += sen;
            byte_rem -= sen;
        }
        
        memset(&data, 0, sizeof(data));
    }
}