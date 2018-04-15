#include "json.h"

int add_to_front(void* value, dict* d) {
  if (value == NULL || d == NULL) return 0;

  node* new = malloc(sizeof(node));
  if (new == NULL) return 0;

  new->value = value;
  new->next = d->head;

  d->head = new;
  d->size++;

  return 1;
}

int add_to_dict(void* value, dict* d) {
  if (value == NULL || d == NULL) return 0;
  if (d->head == NULL) return add_to_front(value, d);

  node* new = malloc(sizeof(node));
  if (new == NULL) return 0;

  new->value = value;
  new->next = NULL;

  node* n = d->head;
  while (n->next != NULL) {
    n = n->next;
  }

  n->next = new;

  d->size++;
 
  return 1;
}

int write_to_json(char* filename, dict* d) {
  
  FILE* fp = fopen(filename, "w+");

  if (fp == NULL) {
    perror("Cannot open file");
  }

  fprintf(fp, "{");

  node* trav = d->head;

  while (trav != NULL) {
    kvp* k       = (kvp*) trav->value;
    int key      = k->key;
    float value  = k->value;

    // printing...
    fprintf(fp, "\"");
    fprintf(fp, "%i", key);
    fprintf(fp, "\"");
    fprintf(fp, ":");
    fprintf(fp, "%f", value);

    trav = trav->next;
    if (trav != NULL) {
      fprintf(fp, ",");
    }

  }

  fprintf(fp, "}");

  fclose(fp);

  return 0;
}

kvp* make_pair(int key, float value) {
  kvp* k = malloc(sizeof(kvp));

  k->key = key;
  k->value = value;

  return k;
}

int update_json_file(char* filename, kvp* k) {
  return 0;
}


int main(int argc, char const *argv[]) {
  
  dict* d = malloc(sizeof(dict));

  kvp* k1 = make_pair(33, 123.1);
  kvp* k2 = make_pair(32, 32.1);

  add_to_dict(k1, d);
  add_to_dict(k2, d);

  write_to_json("out.json", d);


  return 0;
}