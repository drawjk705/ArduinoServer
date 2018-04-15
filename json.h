/** Library of json functions */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

typedef struct KVPair kvp;
struct KVPair {
    void* key;
    void* value;
};

int add_to_dict(kvp* p, dict* d);
char* dict_to_string(dict* d);
int write_to_file(char* filename, dict* d);
dict* read_from_file(char* filename);
kvp* make_pair(void* key, void* value);

char* num_to_string(void* num);