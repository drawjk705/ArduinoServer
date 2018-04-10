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

// Arduino-specific functions
void configure(int fd);

void* get_started(void* p);

void* get_temps(void* p);

float get_avg_temp(void* l);

float get_max_temp(void* l);

float get_min_temp(void* l);

float* read_temp(char* file_name, int fd);

float* strip_letters(char* str);