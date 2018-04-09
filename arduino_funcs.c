#include "arduino_funcs.h"
#include "linkedlist.h"

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
void* get_temps(void* p) {
  
  char* file_name = (char*) p;

  // try to open the file for reading and writing
  // you may need to change the flags depending on your platform
  int fd = open(file_name, O_RDWR | O_NOCTTY | O_NDELAY);
  
  if (fd < 0) {
    perror("Could not open file\n");
    exit(1);
  }
  else {
    printf("Successfully opened %s for reading and writing\n", file_name);
  }

  configure(fd);

  // create LinkedList
  linkedlist* l = malloc(sizeof(linkedlist));

  while (1) {
    float* f = read_temp(file_name, fd);
    
    // add temperature to linked list,
    // to display most recent temperatures
    add_to_tail(f, l);

    // remove the least recently added
    // temperature if size exceeds 100
    if (l->size > 100) {
      remove_from_front(l);
    }
    
    ////////////////////////
    // write to HTML file //
    ////////////////////////

    free(f);
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

  while (1) {

    int index = 0;

    char buf[100];

    while (1) {
        int bytes_read = read(fd, buf, 100);

        for (int i = 0; i < bytes_read; i++) {
            out[index++] = buf[i];
            if (buf[i] == '\n') {
                out[index + 1] = '\0';
                printf("%s\n", out);
                break;
                // out[0] = '\0';
                // break;
            }
        }
    }
  }
  float* f = strip_letters(out);

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
