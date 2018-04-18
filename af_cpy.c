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
    close(ard_fd);
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

  printf("ard_fd is now: %d\n", ard_fd);

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

  dict* d = create_dict();

  packet* pack   = (packet*) p;

  // unpack packet
  char* filename        = pack->filename;
  int ard_fd            = pack->ard_fd;
  char* quit            = pack->quit_ptr;
  pthread_mutex_t* lock = pack->lock;

  char q = *quit;

  while (q != 'q') {
    
    // if connection to Arduino is open
    if (is_open) {
      if (pack->ctrl_signal != '\0') {
        char buf[] = { pack->ctrl_signal };
        printf("%c\n", buf[0]);

        pthread_mutex_lock(lock);
        write(ard_fd, &buf[0], sizeof(buf[0]));
        pack->ctrl_signal = '\0';
        pthread_mutex_unlock(lock);

      }
      if (strcmp(pack->temp_type, "C") == 0) {
        replace_head(d, "C");
      } else {
        replace_head(d, "F");
      }
      char* f_temporary = read_data(filename, ard_fd, lock);
      
      /**** write temperature to file ****/
      
      // get current time
      char* curr_time = get_current_time();
      strcpy(curr_time, curr_time);
      // strip_fat(curr_time);
      // printf("%s\n", curr_time);

      char str_temp[20];
      // convert temperature to string
      if (f_temporary != NULL) {
        // str_temp = num_to_string(f);
        strcpy(str_temp, f_temporary);
        free(f_temporary);
      }
      else {
        strcpy(str_temp, "OFFLINE");
      }

      // printf("%s\n", str_temp);

      // create key-value pair of the 2
      kvp* k = make_pair(curr_time, str_temp);

      // add to dictionary
      add_to_dict(k, d);

      // write dictionary to file
      write_to_json("temperatures.json", d);

      // free(f);
      printf("completed readings\n");
    }
    // if connection is not open
    else {
      close(ard_fd);

      /**** send message to file ****/
      
      // get current time
      char* curr_time = get_current_time();
      strip_fat(curr_time);

      // send offline message
      char* message = "OFFLINE";
      // strcpy(message, "OFFLINE");

      // create key-value pair of the 2
      kvp* k = make_pair(curr_time, message);
      // free(message);

      // add to dictionary
      add_to_dict(k, d);

      // write to file
      write_to_json("temperatures.json", d);

      /******************************/

      printf("Arduino is not connected. Will try to connect\n");
      // try the first port option
      // pthread_mutex_lock(lock);
      int temp = get_started("/dev/ttyACM0");
      if (is_open) {
        // reset filename and ard_fd
        filename       = "/dev/ttyACM0";
        ard_fd         = temp;
        pack->ard_fd   = ard_fd;
        printf("ard_fd = %d vs temp = %d\n", ard_fd, temp);
        printf("new fd = %d\n", pack->ard_fd);
        pack->filename = filename;

        pthread_mutex_lock(lock);
        char buf[] = {'\0'};
        write(ard_fd, &buf[0], sizeof(buf[0]));
        pthread_mutex_unlock(lock);
      }
      // otherwise, try second
      if (!is_open) {
        temp = get_started("/dev/ttyACM1");
        if (is_open) {

          char buf[] = {'\0'};

          // pthread_mutex_lock(lock);
          write(ard_fd, &buf[0], sizeof(buf[0]));
          // pthread_mutex_unlock(lock);

          // reset filename and ard_fd
          filename       = "/dev/ttyACM1";
          ard_fd         = temp;
          pack->ard_fd   = ard_fd;
          printf("ard_fd = %d vs temp = %d\n", ard_fd, temp);
          printf("new fd = %d\n", pack->ard_fd);
          pack->filename = filename;
        }
      }
      // pthread_mutex_unlock(lock);
      if (!is_open) {
        sleep(5);
      }
    }
    sleep(2);
    // pthread_mutex_lock(lock);
    q = *quit;
    // pthread_mutex_unlock(lock);
  }

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
  
  printf("file descriptor is: %d\n", fd);

  char* out = malloc(sizeof(char) * 100); // what output will be
  int index = 0;                          // index to keep track of location in out[]

  char buf[100];                          // buffer to read in data

  int end = 0;                            // flag to determine if are at end of message

  // if have received a certain number of readings
  // with 0 bytes, arduino must have been disconnected
  int zero_count = 0;

  while (end == 0) {

      if (zero_count == 20) {
        printf("Arduino has been disconnected\n");
        is_open = 0;
        free(out);
        return NULL;
      }
      pthread_mutex_lock(lock);
      int bytes_read = read(fd, buf, 100);
      pthread_mutex_unlock(lock);
      if (bytes_read == 0) {
        zero_count++;
      } else {
        zero_count = 0;
      }
      for (int i = 0; i < bytes_read; i++) {
          if (buf[i] == '\n') {
              out[index] = '\0';
              end = 1;
              break;
          }
          out[index++] = buf[i];
      }
  }

  // float f = atof(out);

  // float* f = strip_letters(out);
  printf("%s\n", out);

  return out;
}

// /**
//  * strip letters from the Arduino message
//  * @param  str the Arduino message
//  * @return     solely the temperature (as a float)
//  */
// float* strip_letters(char* str) {

//   float* f = malloc(sizeof(float));

//   char out[10];
//   int at_num = 0;
//   int index = 0;

//   for (int i = 0; i < strlen(str); i++) {
//     // anything that's a number or decimal
//     if ((str[i] >= '0' && str[i] <= '9') || str[i] == '.') {
//         out[index++] = str[i];
//         at_num = 1;
//         if (index == 9) {
//           break;
//         }
//     } else if (at_num != 0) {
//       break;
//     }
//   }
//   out[index] = '\0';      // null terminate

//   *f = atof(out);

//   return f;
// }

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
  // }
  // char* result = read_data(filename, fd);
  // while (strcmp(result, "r") != 0) {
  //   free(result);
  //   write(fd, &c, sizeof(char));
  //   result = read_data(filename, fd);
  // }
  // free(result);
  printf("\n\n\twriting %c to Arduino\n", c);
}




// int main() {


//   int fd = get_started("/dev/ttyACM0");
//   // sleep(10);


//   char buf[3];

//   while (buf[0] != 'q') {
//     scanf("%s", buf);
//     printf("%s\n", buf);
//     write(fd, &buf[0], sizeof(buf[0]));
//   }

//   close(fd);

//   // char buf[1];
//   // buf[0] = 'b';
//   // write(fd, buf, sizeof(buf));

  

//   // printf("writing...\n");
//   // write(fd, buf, sizeof(buf));

//   return 0;
// }
