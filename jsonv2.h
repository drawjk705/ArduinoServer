#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* create_first_pair(char* key, char* value);

void add_kvp(char** kvp, char* key, char* value);

void change_status(char** kvp, char new_status);

void write_to_file(char* kvp, char* filname);

void destroy_kvps(char** kvp);