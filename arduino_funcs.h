/** Arduino-specific functions **/

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "times.h"
#include "json.h"

void configure(int fd);

int check_if_open(int fd);

int get_started(char* file_name);

void* handle_arduino(void* p);

char* read_data(char* file_name, int fd, pthread_mutex_t* lock);

void write_temp_to_file(float* temp);

void write_to_arduino(int fd, char c);
