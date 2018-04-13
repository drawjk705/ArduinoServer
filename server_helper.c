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
 * @return         0 if GET, 1 if POST
 */
int is_get(char* request) {
    
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
        return 0;
    } else if (strcmp(type, "POST") == 0) {
        return 1;
    }
    return -1;
}

/**
 * function to parse the post data
 * @param  request the HTTP request
 * @return         the data that's being posted,
 *                 as retrieved from the bottom
 *                 of the post request
 */
char* get_post(char* request) {

    // create copy of the request,
    // so as not to ruin the original
    char rcpy[strlen(request) + 1];
    strcpy(rcpy, request);

    // delimiter for tokenizing
    const char delim[2] = "\n";
    char* tok = strtok(rcpy, delim);
    // keep on tokenizing until hit blank line
    while (tok != NULL) {
        // indicates blank line
        if (strlen(tok) == 1 && tok[0] == 13) {
            // tokenize one last time, and then break
            tok = strtok(NULL, delim);
            break;    
        }
        tok = strtok(NULL, delim);
    }
    // return the value
    return tok;
}

/**
 * function to parse request portion of HTTP request,
 * so that can figure out which HTML file to access
 * @param  request the full request
 * @return         parsed request
 */
char* get_path(char* request) {

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

/**
 * parse a post request
 * @param  post the request
 * @return      char flag for what to do based on request
 */
char parse_post(char* post) {
    char buf[10];

    for (int i = 0; i < strlen(post); i++) {
        
        buf[i] = post[i];
        if (post[i + 1] == '=') {
            buf[i + 1] = '\0';
            break;
        }
    }
    
    if (strcmp(buf, "red") == 0)    return 'r';
    if (strcmp(buf, "green") == 0)  return 'g';
    if (strcmp(buf, "blue") == 0)   return 'b';


    return '\0';
}