#include "jsonv2.h"

char* create_first_pair(char* key, char* value) {

    int total_len = (strlen(key) + strlen(value) + 6);

    char* kvp = malloc((strlen(key) + strlen(value) + 6) * sizeof(char));

    sprintf(kvp, "\"%s\":\"%s\"", key, value);
    kvp[total_len] = '\0';

    return kvp;
}

void change_status(char** kvp, char new_status) {
    
    if (*kvp == NULL) {
        return;
    }
    (*kvp)[10] = new_status;
}

void add_kvp(char** kvp, char* key, char* value) {

    int total_len = (strlen(key) + strlen(value) + 7);

    *kvp = realloc(*kvp, (strlen(*kvp) + total_len) * sizeof(char));

    char* temp = create_first_pair(key, value);

    strcat(*kvp, ",");

    strcat(*kvp, temp);

    free(temp);
}

void destroy_kvps(char** kvp) {
    if (*kvp != NULL) {
        free(*kvp);
    }
}

void write_to_file(char* kvp, char* filename) {

    FILE* fp = fopen(filename, "w+");
    if (fp == NULL) {
        perror("Cannot open file");
        return;
    }

  // print opening {
  fprintf(fp, "{");
  fprintf(fp, "%s", kvp);
  fprintf(fp, "}");


  fclose(fp);

}

// int main(int argc, char const *argv[])
// {
//     char* kvp = create_first_pair("hello", "world");
//     add_kvp(&kvp, "helloo", "again");
//     write_to_file(kvp, "test.json");
//     destroy_kvps(&kvp);

//     return 0;
// }