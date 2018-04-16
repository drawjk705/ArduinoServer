#include "arduino_funcs.h"
#include "server_helper.h"

// flag to determine if arduino is open or not
// for reading and writing
int is_open;    // 0 if not open, 1 if open

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
 * function to get Arduino ready for reading
 * @param p pointer to filename
 */
int get_started(char* file_name) {

  // try to open the file for reading and writing
  // you may need to change the flags depending on your platform
  int ard_fd = open(file_name, O_RDWR | O_NOCTTY | O_NDELAY);
  
  if (ard_fd < 0) {
    printf("Cannot open file\n");
    is_open = 0;
    return 0;
    // perror("Could not open file\n");
    // exit(1);
  }
  else {
    printf("Successfully opened %s for reading and writing\n", file_name);
    is_open = 1;
  }

  configure(ard_fd);

  return ard_fd;
}

/**
 * function to keep on getting the
 * current temperature from the Arduino
 *
 * will also write the temperature to a json file
 * 
 * @param p packet with relevant data
 */
void* get_temps(void* p) {

  // char* filename;
  // int ard_fd;
  // is_open = 0;

  dict* d = malloc(sizeof(dict));

  printf("In gettemps\n");

  packet* pack   = (packet*) p;

  // unpack packet
  char* filename = pack->filename;
  int ard_fd     = pack->ard_fd;
  char* quit     = pack->quit_ptr;

  // read through a few times to get
  // rid of any potential garbage that's
  // being outputted
  if (is_open) {
    for (int i = 0; i < 10; i++) {
      read_temp(filename, ard_fd);
    }
  }

  while (*quit != 'q') {
    printf("%c\n", *quit);
    
    // if connection to Arduino is open
    if (is_open) {
      float* f = read_temp(filename, ard_fd);
      
      /**** write temperature to file ****/
      
      // get current time
      char* curr_time = get_current_time();
      strip_fat(curr_time);

      // convert temperature to string
      char* str_temp = num_to_string(f);

      // create key-value pair of the 2
      kvp* k = make_pair(curr_time, str_temp);

      // add to dictionary
      add_to_dict(k, d);

      // write dictionary to file
      write_to_json("temperatures.json", d);

      // free(f);
      printf("completed readings\n");
      sleep(2);
    }
    // if connection is not open
    else {

      /**** send message to file ****/
      
      // get current time
      char* curr_time = get_current_time();
      strip_fat(curr_time);

      // send offline message
      char* message = malloc(sizeof(char) * (strlen("OFFLINE") + 1));
      strcpy(message, "OFFLINE");

      // create key-value pair of the 2
      kvp* k = make_pair(curr_time, message);

      // add to dictionary
      add_to_dict(k, d);

      // write to file
      write_to_json("temperatures.json", d);

      /******************************/

      printf("Arduino is not connected. Will try to connect\n");
      // try the first port option
      int temp = get_started("/dev/tty/ACM0");
      if (is_open) {
        // reset filename and ard_fd
        filename     = "/dev/tty/ACM0";
        ard_fd       = temp;
        pack->ard_fd = ard_fd;
      }
      // otherwise, try second
      if (!is_open) {
        temp = get_started("/dev/tty/ACM1");
        if (is_open) {
          // reset filename and ard_fd
          filename     = "/dev/tty/ACM1";
          ard_fd       = temp;
          pack->ard_fd = ard_fd;
        }
      }
      if (is_open) {
        for (int i = 0; i < 10; i++) {
          read_temp(filename, ard_fd);
        }
      } else {
        sleep(5);
      }
    }
  }

  clear_dictionary(d);

  pthread_exit(NULL);
}

/**
 * funciton for single read of temperature from Arduino
 * @param  file_name Arduino filename
 * @param  fd        file descriptor
 * @return           the temperature as a float, without any surrounding text
 */
float* read_temp(char* file_name, int fd) {
  
  char out[100]; // what output will be
  int index = 0; // index to keep track of location in out[]

  char buf[100]; // buffer to read in data

  int end = 0;   // flag to determin if are at end of message

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

/**
 * strip letters from the Arduino message
 * @param  str the Arduino message
 * @return     solely the temperature (as a float)
 */
float* strip_letters(char* str) {

  float* f = malloc(sizeof(float));

  char out[10];
  int at_num = 0;
  int index = 0;

  for (int i = 0; i < strlen(str); i++) {
    // anything that's a number or decimal
    if ((str[i] >= '0' && str[i] <= '9') || str[i] == '.') {
        out[index++] = str[i];
        at_num = 1;
        if (index == 9) {
          break;
        }
    } else if (at_num != 0) {
      break;
    }
  }
  out[index] = '\0';      // null terminate

  *f = atof(out);

  return f;
}

/**
 * send message to Arduino
 * @param fd Arduino file descriptor
 * @param c  message to send
 */
void write_to_arduino(int fd, char c) {
  if (!is_open) {
    printf("Arduino is not open for writing\n");
    return;
  }
  if (write(fd, &c, sizeof(char)) != sizeof(char)) {
    perror("Could not write to Arduino");
    exit(1);
  }
  printf("writing %c to Arduino\n", c);
}

/**
 * check if file descriptor is still open
 * --> will be used by server to report on
 *     arduino status
 * @param  fd file descriptor in question
 * @return    0 if no, 1 if yes
 */
int check_if_open(int fd) {
  if (fcntl(fd, F_GETFL) < 0 && errno == EBADF) {
    is_open = 0;
    return 0;
  } else {
    is_open = 1;
    return 1;
  }
}