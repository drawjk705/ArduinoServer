#include "json.h"

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
  node* prev;

  while (trav != NULL) {
    free(((kvp*)trav->value)->key);
    free(((kvp*)trav->value)->value);
    prev = trav;
    trav = trav->next;
    free(prev);
  }
  free(d);
}


/**
 * simple (doesn't cover all cases) way to convert number
 * (either an int or a float) to a string
 * @param  num the number to convert
 * @return     string representation of that number
 */
char* num_to_string(void* num) {

  // flag to check if the number is an int vs. a float
  int is_int = 0;

  // count of digits before the decimal point
  int before_dec = 0;

  // try to cast to float to int
  int temp = *(float*) num;
  if (temp == 0) {
    // if the cast fails, the number is an int
    temp = *(int*) num;
    is_int = 1;
  }

  // count digits before decimal
  while (temp > 0) {
    temp /= 10;
    before_dec++;
  }

  // count digits following decimal
  int after_dec = 0;
  float temp2 = *(float*) num;
  while (((int) temp2 % 10) != 0) {
    temp2 *= 10;
    after_dec++;
  }
  
  // get full number length
  int num_len = before_dec + after_dec + 1;

  // make string
  char* dest = malloc(sizeof(char) * (num_len + 1));

  // get the full non-float representation of the number
  // i.e., no decimal
  int full_num = 0;
  if (!is_int) {
    full_num = (int) temp2;
  }
  else {
    full_num = *(int*) num;
  }
  // insert digits backwards (from 1s to higher places)
  dest[num_len] = '\0';
  for (int i = num_len - 1; i >= 0; i--) {
    if (i == before_dec) {
      if (!is_int) {
        dest[i] = '.';
      }
    } else {
      int val = full_num % 10;
      dest[i] = '0' + val;
      full_num /= 10;
    }
  }
  dest[num_len] = '\0';

  return dest;
}