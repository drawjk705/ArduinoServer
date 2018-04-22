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
  
  if (ard_fd <= 0) {
    close(ard_fd);
    is_open = 0;
    printf("could not open\n");
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
  pthread_mutex_t* lock = pack->lock;

  int ard_fd = -1;
  char* filename = "/dev/ttyACM0";

  dict* d = create_dict();

  pthread_mutex_lock(lock);
  int is_quit = pack->quit_flag;
  pthread_mutex_unlock(lock);

  while (!is_quit) {

    pthread_mutex_lock(lock);
    if (pack->is_Celsius) {
      replace_head(d, "C");
    } else {
      replace_head(d, "F");
    }
    pthread_mutex_unlock(lock);

    if (is_open) {

      if (pack->ctrl_signal != '\0') {
        char sig[3];
        pthread_mutex_lock(lock);
        sig[0] = pack->ctrl_signal;
        pthread_mutex_unlock(lock);

        sig[1] = '\n';
        sig[2] = '\0';
        printf("writing %s to arduino\n", sig);
        pthread_mutex_lock(lock);
        write(ard_fd, sig, strlen(sig));
        pthread_mutex_unlock(lock);
        sleep(3);
        strcpy(sig, "w\n");
        pthread_mutex_lock(lock);
        write(ard_fd, sig, strlen(sig));
        pack->ctrl_signal = '\0';
        pthread_mutex_unlock(lock);
      }

      char* time = get_current_time();

      pthread_mutex_lock(lock);
      char* temperature = read_data(filename, ard_fd, NULL);
      pthread_mutex_unlock(lock);

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
    else if (!is_open) {
      close(ard_fd);

      printf("Arduino is offline\n");
      // try to open Arduino
      pthread_mutex_lock(lock);
      ard_fd = get_started("/dev/ttyACM0");
      pthread_mutex_unlock(lock);
      if (!is_open) {
        sleep(5);
        pthread_mutex_lock(lock);
        ard_fd = get_started("/dev/ttyACM1");
        pthread_mutex_unlock(lock);
        if (is_open) {
          filename = "dev/ttyACM1";
        }
      } else {
        filename = "dev/ttyACM0";
      }
    }
    sleep(2);
    pthread_mutex_lock(lock);
    is_quit = pack->quit_flag;
    pthread_mutex_unlock(lock);
    if (is_quit) {
      break;
    }
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
      printf("\t\t%d bytes read from arduino\n", bytes_read);

      if (bytes_read == 0) { 
        zero_count++;
      } else {
        zero_count = 0;
      }
      if (zero_count > 20) {
          is_open = 0;
          return NULL;
      }
      if (bytes_read < 0) {
        sleep(2);
      }
      for (int i = 0; i < bytes_read; i++) {
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


// int main() {

//   int ard_fd = get_started("/dev/ttyACM0");
//   if (!is_open) {
//     ard_fd = get_started("/dev/ttyACM1");
//     if (is_open) {
//       // int filename = "dev/ttyACM1";
//     }
//   } else {
//     // filename = "dev/ttyACM0";
//   }

//   // char c = 'b';

//   // sleep(5);

//   // printf("writing %c\n", c);

//   // write(ard_fd, &c, sizeof(char));




//   char buf[3];
//   while (1) {
//     int bytes_read = read(STDIN_FILENO, buf, (sizeof(buf) - 1));
//     buf[bytes_read] = '\0';
//     if (buf[0] == 'q') {
//       break;
//     }
//     printf("read %d bytes, and wrote %s\n", bytes_read, buf);
//     write(ard_fd, buf, strlen(buf));
//   }
//   close(ard_fd);

// }

