/* Header file for helper functions for server.c */

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef struct Packet packet;
struct Packet
{
    struct sockaddr_in client_addr;
    int fd;
};

void* handle_connection(void* p);

int parse_request(char* request);

char* parse_get(char* request);

char* parse_post(char* request);

char* read_html_file(char* filename);

void* close_server(void* p);