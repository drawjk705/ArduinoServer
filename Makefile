CC = clang
ARGS = -Wall -g -lpthread

all: read server ard

# read: read_usb.c
# 	$(CC) -o read $(ARGS) read_usb.c


## making .o file for server_helper
server_helper: server_helper.c
	$(CC) -c $(ARGS) server_helper.c

## making .o file for get_temp
ard: arduino_funcs.c
	$(CC) -c $(ARGS) arduino_funcs.c

server: server_helper ard server.o
	$(CC) -o server $(ARGS) server.c arduino_funcs.o server_helper.o

clear:
	rm -rf *.o server get_temp
