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

void configure(int fd);

int fd_still_open(int fd);

int get_started(char* file_name);

void* get_temps(void* p);

float get_avg_temp(void* l);

float get_max_temp(void* l);

float get_min_temp(void* l);

float* read_temp(char* file_name, int fd);

float* strip_letters(char* str);

void write_temp_to_file(float* temp);

void write_to_arduino(int fd, char c);