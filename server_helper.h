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
#include "linkedlist.h"

typedef struct Packet packet;
struct Packet
{
    struct sockaddr_in client_addr;
    int fd;
    int ard_fd;
    linkedlist** l;
    char* filename;
    char* quit_ptr;
};

void* handle_connection(void* p);

int is_get(char* request);

char* get_path(char* request);

char* get_post(char* request);

char* parse_post(char* post);

char* read_html_file(char* filename);

void* close_server(void* p);