#include "times.h"

char* get_current_time() {
    
    time_t t;
    time(&t);

    char* out = ctime(&t);

    strip_fat(out);
    // printf("%s\n", out);

    return out;
}

void strip_fat(char* time) {

    char* time_cpy = malloc(sizeof(char) * strlen(time));
    strcpy(time_cpy, time);
    char* delim = " ";

    char* token = strtok(time_cpy, delim);

    while (strlen(token) != 8) {
        token = strtok(NULL, delim);
    }
    strcpy(time, token);
    free(time_cpy);
}
