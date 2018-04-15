/** Library of json functions */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Node node;
struct Node {
  void* value;
  node* next;
};

typedef struct Dictonary dict;
struct Dictonary {
  node* head;
  int size;  
};

int add_to_dict(void* value, dict* d);

char* dict_to_string(dict* d);

typedef struct KVPair kvp;
struct KVPair {
    int key;
    float value;
};

int write_to_file(char* filename, dict* d);        // add to file if file exists
int update_file(char* filename, kvp* d);

kvp* make_pair(int key, float value);
char* kvp_to_string(kvp* k);

void my_itoa(int x, char* dest);