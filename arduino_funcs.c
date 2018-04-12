#include "arduino_funcs.h"
#include "server_helper.h"

float get_max_temp(void* l) {
  float max = 0;
  linkedlist* ll = (linkedlist*) l;
  float** array = (float**) to_array(ll);
  for (int i = 0; i < ll->size; i++) {
    if (*(array[i]) > max) {
      max = *(array[i]);
    }
  }
  return max;
}

float get_min_temp(void* l) {
  float min = 999;
  linkedlist* ll = (linkedlist*) l;
  float** array = (float**) to_array(ll);
  for (int i = 0; i < ll->size; i++) {
    if (*(array[i]) < min) {
      min = *(array[i]);
    }
  } 
  return min;
}

float get_avg_temp(void* l) {
  float total = 0;
  linkedlist* ll = (linkedlist*) l;
  float** array = (float**) to_array(ll);
  for (int i = 0; i < ll->size; i++) {
    total += *(array[i]);
  }
  return total / (float) ll->size;
}


/*
This code configures the file descriptor for use as a serial port.
*/
void configure(int fd) {
  struct  termios pts;
  tcgetattr(fd, &pts);
  cfsetospeed(&pts, 9600);   
  cfsetispeed(&pts, 9600);   
  tcsetattr(fd, TCSANOW, &pts);
}

/**
 * function to get temperature from Arduino
 * @param p pointer to filename
 */
int get_started(void* p) {

  // unpack packet
  packet* pack = malloc(sizeof(packet));

  
  char* file_name = (char*) p;

  // try to open the file for reading and writing
  // you may need to change the flags depending on your platform
  int ard_fd = open(file_name, O_RDWR | O_NOCTTY | O_NDELAY);
  
  if (ard_fd < 0) {
    perror("Could not open file\n");
    exit(1);
  }
  else {
    printf("Successfully opened %s for reading and writing\n", file_name);
  }

  configure(ard_fd);

  return ard_fd;
}

void* get_temps(void* p) {

  packet* pack = (packet*) p;

  // unpack packet
  linkedlist** l = pack->l;
  char* file_name = pack->filename;
  int ard_fd = pack->ard_fd;
  char* quit = pack->quit_ptr;

  // do a few times to get rid of garbage
  for (int i = 0; i < 10; i++) {
    read_temp(file_name, ard_fd);
  }
  // int i = 100;
  while (*quit != 'q') {
    // sleep(10);
    float* f = read_temp(file_name, ard_fd);
    
    // add temperature to linked list,
    // to display most recent temperatures
    add_to_tail(f, *l);

    // remove the least recently added
    // temperature if size exceeds 100
    // if ((*l)->size > 100) {
    //   remove_from_front(*l);
    // }
    
    ////////////////////////
    // write to HTML file //
    ////////////////////////

    free(f);
    printf("completed readings\n");
    sleep(2);
    // printf("%d\n", i--);
  }



  pthread_exit(NULL);
}

/**
 * funciton for single read of temperature from Arduino
 * @param  file_name Arduino filename
 * @param  fd        file descriptor
 * @return           the temperature as a float, without any surrounding text
 */
float* read_temp(char* file_name, int fd) {
  /*
    Write the rest of the program below, using the read and write system calls.
  */
  
  char out[100];

  int index = 0;

  char buf[100];

  int end = 0;

  while (end == 0) {
      int bytes_read = read(fd, buf, 100);

      for (int i = 0; i < bytes_read; i++) {
          out[index++] = buf[i];
          if (buf[i] == '\n') {
              out[index + 1] = '\0';
              // printf("%s\n", out);
              end = 1;
              break;
          }
      }
  }
  float* f = strip_letters(out);
  printf("%f\n", *f);

  return f;

}

float* strip_letters(char* str) {

  float* f = malloc(sizeof(float));

  int start = 0;
  int end = 0;

  for (int i = 0; i < strlen(str); i++) {
    // anything that's a number or decimal
    if ((str[i] >= 49 && str[i] <= 57) || str[i] == 46) {
      if (start == 0) {
        start = i;
      }
    } else if (start != 0) {
      end = i;
      break;
    }
  }
  if (end == 0) {
    end = strlen(str) - 1;
  }

  char out[end - start];

  for (int i = 0; i < end - start; i++) {
    out[i] = str[i + start];
  }

  *f = atof(out);

  return f;
}

void write_to_arduino(int fd, char c) {
  if (write(fd, &c, sizeof(char)) != sizeof(char)) {
    perror("Could not write to Arduino");
    exit(1);
  }
  printf("writing %c to Arduino\n", c);
}

void write_temp_to_file(float* temp) {

  FILE* f = fopen("temperatures.txt", "r");

  // get file length
  int file_len = fseek(f, 0, SEEK_END);

  char* buff = malloc(sizeof(char) * (file_len + 1));

  // reset location in file
  fseek(f, 0, SEEK_SET);

  // read file into buffer
  fread(buff, sizeof(char), file_len, f);

  fclose(f);

  f = fopen("temperatures.txt", "w");

  fwrite(temp, sizeof(float), 1, f);
  fwrite(buff, sizeof(char), strlen(buff), f);

  fclose(f);

}