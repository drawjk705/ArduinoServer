CC = clang
ARGS = -Wall -g -lpthread

all: read server get_temp

# read: read_usb.c
# 	$(CC) -o read $(ARGS) read_usb.c


## making .o file for server_helper
server_helper: server_helper.c
	$(CC) -c $(ARGS) server_helper.c

## making .o file for get_temp
get_temp: get_temp.c
	$(CC) -c $(ARGS) get_temp.c

server: server_helper.o get_temp.o server.o
	$(CC) -o server $(ARGS) server.c get_temp.o server_helper.o

clear:
	rm -rf server get_temp
