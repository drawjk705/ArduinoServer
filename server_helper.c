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
#include "server_helper.h"

/**
 * parse HTTP request to see if it's a GET or POST
 * @param  request the HTTP request
 * @return         1 if GET, 2 if POST
 */
int parse_request(char* request) {
    
    char type[5];

    for (int i = 0; i < strlen(request); i++) {
        if (request[i] == ' ') {
            type[i] = '\0';
            break;
        } else {
            type[i] = request[i];
        }
    }

    if (strcmp(type, "GET") == 0) {
        return 1;
    } else if (strcmp(type, "POST") == 0) {
        printf("%s\n", type);
        printf("%s\n", request);
        return 2;
    }
    return 0;
}

char* parse_post(char* request) {
    return NULL;
}



/**
 * function to parse request portion of HTTP request,
 * so that can figure out which HTML file to access
 * @param  request the full request
 * @return         parsed request
 */
char* parse_get(char* request) {

    // find appropriate indeces to "slice"
    int start = 0, end = 0;
    for (int i = 0; i < strlen(request); i++) {
        if (request[i] == '/' && start == 0) {
            start = i + 1;
        }
        if (request[i] == ' ' && start > 0) {
            end = i;
            break;
        }
    }

    // copy over request
    char* out = malloc(sizeof(char) * (end - start + 1));

    for (int i = 0; i < end - start; i++) {
        out[i] = request[i + start];
    }
    // null terminate
    out[end - start] = '\0';
    return out;
}

/**
 * function to read in HTML file, as
 * requested in the HTTP request
 * @param  filename filename, as parsed
 * @return          the filetext
 */
char* read_html_file(char* filename) {
    
    // create file pointer
    FILE* fp = fopen(filename, "r");

    // get length of file
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);

    // malloc
    char* out = malloc(sizeof(char) * (len + 1));
    if (out == NULL) {
        return NULL;
    }

    // rewind fp to start
    rewind(fp);

    // read file into out
    fread(out, sizeof(char), len - 1, fp);

    // null terminate
    out[len] = '\0';

    // close file, and return
    fclose(fp);
    return out;
}