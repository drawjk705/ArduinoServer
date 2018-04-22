#include "json.h"

dict* create_dict() {
  dict* d = malloc(sizeof(dict));
  // char* temp = malloc(sizeof(char) * (strlen("Temp") + 1));
  // strcpy(temp, "Temp");
  kvp* head = make_pair("Status", "C");
  d->head = malloc(sizeof(node*));
  d->head->value = head;
  return d;
}

int replace_head(dict* d, char* temp) {
  
  if (d == NULL || d->head == NULL) {
    return -1;
  }

  // char* t = malloc(sizeof(char) * (strlen("Temp") + 1));
  // strcpy(t, "Temp");

  kvp* new_head = make_pair("Status", temp);

  kvp* old_head = d->head->value;

  free(old_head);

  d->head->value = new_head;

  return 0;
}

/**
 * add to dictionary -- which is essentially a linked list
 * @param  p key-value pait to add
 * @param  d dictionary to which to add
 * @return   0 if fail, 1 if success
 */
int add_to_dict(kvp* p, dict* d) {
  if (p == NULL || d == NULL) {
    return 0;
  }
  if (d->head == NULL) {
    node* new   = malloc(sizeof(node));
    if (new == NULL) {
      return 0;
    }
    new->value  = p;
    new->next   = NULL;
    d->head     = new;
  }
  else {
    node* new   = malloc(sizeof(node));
    if (new == NULL) {
      return 0;
    }  

    new->value  = p;
    new->next   = NULL;

    node* n = d->head;
    while (n->next != NULL) {
      n = n->next;
    }

    n->next = new;
  }

  d->size++;
 
  return 1;
}

/**
 * take a dictionary and write it to file
 * @param  filename filename to which to write the dictionary
 * @param  d        dictionary to write
 * @return          1 if success, 0 if fail
 */
int write_to_json(char* filename, dict* d) {
  
  // open file
  FILE* fp = fopen(filename, "w+");
  if (fp == NULL) {
    perror("Cannot open file");
    return 0;
  }

  // print opening {
  fprintf(fp, "{");

  // trav along
  node* trav = d->head;

  while (trav != NULL) {

    // go through each kvp
    kvp* k = (kvp*) trav->value;

    // print appropriate quotation marks
    fprintf(fp, "\"");
    fprintf(fp, "%s", (char*) k->key);
    fprintf(fp, "\"");
    fprintf(fp, ":");
    
    fprintf(fp, "\"");
    // print value string; omit quotation marks
    for (int i = 0; i < strlen((char*)k->value); i++) {
      if (((char*)k->value)[i] != '"') {
        fprintf(fp, "%c", ((char*)k->value)[i]);
      }
    }
    // print closing quotation mark
    fprintf(fp, "\"");

    trav = trav->next;
    // print , where appropriate
    if (trav != NULL) {
      fprintf(fp, ",");
    }

  }

  // close off
  fprintf(fp, "}");

  fclose(fp);

  return 1;
}

/**
 * read a dictionary from a .json file
 * @param  filename [description]
 * @return          [description]
 */
dict* read_from_file(char* filename) {

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("file does not exist");
    exit(1);
  }

  // create dictionary pointer
  dict* d;

  // buffer to hold each char
  char buf;

  // buffer for key data
  char* key_buf = malloc(sizeof(char) * 20);
  int key_ind   = 0;

  // buffer for value data
  char* val_buf = malloc(sizeof(char) * 20);
  int val_ind   = 0;

  // flag to determine if are at a value or not
  int is_val    = 0;
    
  // "stack" to keep track of quotation marks
  char stack[1];
  stack[0] = '\0';            // set the stack

  // keep on reading...
  while (fread(&buf, sizeof(char), 1, fp) == 1) {

    // when are at the start of a dictionary
    if (buf == '{') {
      d = malloc(sizeof(dict));
      d->head = NULL;
      d->size = 0;
    }
    // when are at end of kvp
    else if (buf == '}' || (buf == ',' && stack[0] == '\0')) {
      // null-terminate value
      val_buf[val_ind] = '\0';
      
      // copy strings
      char* k_cpy = malloc(sizeof(char) * (strlen(key_buf) + 1));
      char* v_cpy = malloc(sizeof(char) * (strlen(val_buf) + 1));
      
      strcpy(k_cpy, key_buf);
      strcpy(v_cpy, val_buf);
      
      // make the kvp
      kvp* k = make_pair(k_cpy, v_cpy);
      // add to the dictionary
      add_to_dict(k, d);
      // continue if appropriate
      if (buf == ',') {
        is_val = 0;

        val_ind= 0;
        key_buf[0] = '\0';
        val_buf[0] = '\0';

      }
      // terminate, if not
      else {
        break;
      }
    }
    // balancing quotation marks
    else if (buf == '"') {
      if (stack[0] == '\0') {
        stack[0] = '"';
      } else {
        stack[0] = '\0';
      }
    }
    // time to write
    else {
      if (buf == ':' && stack[0] == '\0') {
        // set flag
        is_val = 1;
        // null-terminate key
        key_buf[key_ind] = '\0';
        // reset key index
        key_ind = 0;
      }
      else {
        // store to buffers
        if (!is_val) {
          key_buf[key_ind++] = buf;
          if (key_ind >= strlen(key_buf)) {
            key_buf = realloc(key_buf, sizeof(char) * strlen(key_buf) * 2);
          }
        } else {
          val_buf[val_ind++] = buf;
          if (val_ind >= strlen(val_buf)) {
            val_buf = realloc(val_buf, sizeof(char) * strlen(val_buf) * 2);
          }
        }
      }
    }
  }
  
  return d;
}


/**
 * make a key-value pair
 * @param  key   key of the pair
 * @param  value value of the pair
 * @return       kvp pointer
 */
kvp* make_pair(void* key, void* value) {

  kvp* k = malloc(sizeof(kvp));

  k->key = key;
  k->value = value;

  return k;
}

void clear_dictionary(dict* d) {

  node* trav = d->head;
  
  // decapitate
  free(d->head->value);
  free(d->head);

  // for the rest of the dict...
  trav = trav->next;
  node* prev;

  while (trav != NULL) {
    // free(((kvp*)trav->value)->value);
    prev = trav;
    trav = trav->next;
    void* key = ((kvp*)prev->value)->key;
    free(key);
    free(prev->value);
    free(prev);
  }
  free(d);
}
