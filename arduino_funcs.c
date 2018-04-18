#include "arduino_funcs.h"
#include "server_helper.h"

int is_open = 0;

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
int get_started(char* filename) {

  // try to open the file for reading and writing
  // you may need to change the flags depending on your platform
  int ard_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);
  
  if (ard_fd < 0) {
    close(ard_fd);
    is_open = 0;
    // perror("Could not open file\n");
    // exit(1);
    return -1;
  }
  else {
    printf("Successfully opened %s for reading and writing\n", filename);
    is_open = 1;
  }

  configure(ard_fd);
  printf("ard_fd = %d\n", ard_fd);

  return ard_fd;
}

void* handle_arduino(void* p) {


  packet* pack = (packet*) p;

  int ard_fd = -1;
  char* filename = "/dev/ttyACM0";

  dict* d = create_dict();

  while (!pack->quit_flag) {

    if (pack->is_Celsius) {
      replace_head(d, "C");
    } else {
      replace_head(d, "F");
    }

    if (is_open) {

      if (pack->ctrl_signal != '\0') {
        printf("writing %c to arduino\n", pack->ctrl_signal);
        write(ard_fd, &(pack->ctrl_signal), sizeof(char));
        pack->ctrl_signal = '\0';
      }
      if (pack->quit_flag) {
        break;
      }
      char* time = get_current_time();

      char* temperature = read_data(filename, ard_fd, NULL);
      char* value = malloc(sizeof(char) * 20);
      if (temperature == NULL) {
        strcpy(value, "OFFLINE");
      } else {
        strcpy(value, temperature);
        free(temperature);
      }
      kvp* k = make_pair(time, value);
      add_to_dict(k, d);
      write_to_json("output.json", d);
    }
    else {
      printf("Arduino is offline\n");
      // try to open Arduino
      ard_fd = get_started("/dev/ttyACM0");
      if (!is_open) {
        ard_fd = get_started("/dev/ttyACM1");
        if (is_open) {
          filename = "dev/ttyACM1";
        }
      } else {
        filename = "dev/ttyACM0";
      }
    }
    sleep(2);
  }
  close(ard_fd);

  clear_dictionary(d);

  pthread_exit(NULL);
}

/**
 * function for single read of temperature from Arduino
 * @param  file_name Arduino filename
 * @param  fd        file descriptor
 * @return           the temperature as a float, without any surrounding text
 */
char* read_data(char* file_name, int fd, pthread_mutex_t* lock) {
  /*
    Write the rest of the program below, using the read and write system calls.
  */
 
  printf("file descriptor is: %d\n", fd);
  
  char out[100];
  int index = 0;
  char buf[100];
  int end = 0;
  int zero_count = 0;

  while (end == 0) {
      int bytes_read = read(fd, buf, 100);
      if (bytes_read == 0) { 
        zero_count++;
        if (zero_count > 20) {
          close(fd);
          is_open = 0;
          return NULL;
        }
      }
      for (int i = 0; i < bytes_read; i++) {
        zero_count = 0;
          if (buf[i] == '\n') {
              out[index + 1] = '\0';
              printf("%s\n", out);
              end = 1;
              break;
          }
          out[index++] = buf[i];
      }
  }
  char* temp = malloc(sizeof(char) * (strlen(out) + 1));
  strcpy(temp, out);
  
  return temp;
}
