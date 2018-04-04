CC = clang
ARGS = -Wall -g -lpthread

all: read server get_temp

read: read_usb.c
	$(CC) -o read $(ARGS) read_usb.c

server: server.c
	$(CC) -o server $(ARGS) server.c get_temp.c

get_temp: get_temp.c
	$(CC) -c $(ARGS) get_temp.c

clear:
	rm -rf read server get_temp
