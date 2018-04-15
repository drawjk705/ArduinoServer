#include "json.h"

int add_to_dict(kvp* p, dict* d) {
  if (p == NULL || d == NULL) {
    return 0;
  }
  if (d->head == NULL) {
    node* new   = malloc(sizeof(node));
    new->value  = p;
    new->next   = NULL;
    d->head     = new;
  }
  else {
    node* new         = malloc(sizeof(node));
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

int write_to_json(char* filename, dict* d) {
  
  FILE* fp = fopen(filename, "w+");
  if (fp == NULL) {
    perror("Cannot open file");
  }
  fprintf(fp, "{");

  node* trav = d->head;

  while (trav != NULL) {
    kvp* k       = (kvp*) trav->value;
    void* key    = k->key;
    void* value  = k->value;

    // printing...
    fprintf(fp, "\"");
    fprintf(fp, "%s", (char*) key);
    fprintf(fp, "\"");
    fprintf(fp, ":");
    
    fprintf(fp, "\"");
    if (((char*)value)[0] == '"') {
      for (int i = 0; i < strlen((char*)value); i++) {
        if (((char*)value)[i] != '"') {
          fprintf(fp, "%c", ((char*)value)[i]);
        }
      }
    }
    else {
      char* string_rep = num_to_string(value);
      fprintf(fp, "%s", string_rep);
      free(string_rep);
    }
    fprintf(fp, "\"");

    trav = trav->next;
    if (trav != NULL) {
      fprintf(fp, ",");
    }

  }

  // close off
  fprintf(fp, "}");

  fclose(fp);

  return 0;
}


dict* read_from_file(char* filename) {

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    return NULL;
  }

  dict* d;

  char buf;
  char* key_buf = malloc(sizeof(char) * 20);
  int key_ind   = 0;

  char* val_buf = malloc(sizeof(char) * 20);
  int val_ind   = 0;

  int is_val    = 0;

  while (fread(&buf, sizeof(char), 1, fp) == 1) {

    // starting dictionary
    if (buf == '{') {
      d = malloc(sizeof(dict));
      d->head = NULL;
    }
    // ending key-value pair or dictionary
    else if (buf == ',' || buf == '}') {
      val_buf[val_ind] = '\0';
      is_val = 0;
      kvp* temp = make_pair(key_buf, val_buf);  // this will free() key_buf & val_buf
      add_to_dict(temp, d);                     // this will free() temp

      // if are continuing dictionary
      if (buf == ',') {
        char* key_buf = malloc(sizeof(char) * 20);
        int key_ind   = 0;

        char* val_buf = malloc(sizeof(char) * 20);
        int val_ind   = 0;
      }
      // otherwise
      else {
        break;
      }
    }
    // end of key
    else if (buf == ':') {
      key_buf[key_ind] = '\0';
      is_val = 1;
    }
    // writing key or value
    else {
      // if key
      if (!is_val) {
        // ignore quotation marks
        if (buf == '"') {
          // do nothing
        }
        else {
          // write into key_buf
          key_buf[key_ind++] = buf;
          // realloc() if necessary
          if (key_ind == sizeof(key_buf)) {
            key_buf = realloc(key_buf, sizeof(key_buf) * 2);
          }
        }
      }
      // if value
      else {
        // write into val_buf
        val_buf[val_ind++] = buf;
        // realloc() if necessary
        if (val_ind == sizeof(val_buf)) {
          val_buf = realloc(val_buf, sizeof(val_buf) * 2);
        }
      }
    }
  }
  return d;
}




kvp* make_pair(void* key, void* value) {

  kvp* k = malloc(sizeof(kvp));

  k->key = key;
  k->value = value;

  return k;
}


int main(int argc, char const *argv[]) {

  dict* d = malloc(sizeof(dict));

  char* curr_time = "hello";
  float temp = 24.1;

  kvp* k1 = make_pair(curr_time, &temp);

  add_to_dict(k1, d);

  write_to_json("out.json", d);

  dict* d2 = read_from_file("out.json");
  write_to_json("out2.json", d2);


  return 0;
}

/**
 * simple (doesn't cover all cases) way to convert number
 * (either an int or a float) to a string
 * @param  num the number to convert
 * @return     string representation of that number
 */
char* num_to_string(void* num) {

  int is_int = 0;
  int before_dec = 0;
  int temp = *(float*) num;
  if (temp == 0) {
    temp = *(int*) num;
    is_int = 1;
  }
  while (temp > 0) {
    temp /= 10;
    before_dec++;
  }

  int after_dec = 0;
  float temp2 = *(float*) num;
  while (((int) temp2 % 10) != 0) {
    temp2 *= 10;
    after_dec++;
  }
  
  int num_len = before_dec + after_dec + 1;
  char* dest = malloc(sizeof(char) * (num_len + 1));

  int full_num = 0;
  if (!is_int) {
    full_num = (int) temp2;
  }
  else {
    full_num = *(int*) num;
  }
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