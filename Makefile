CC = clang
ARGS = -Wall -g -lpthread

all: linkedlist server_helper ard server

# making .o file for linkedlist.c
linkedlist: linkedlist.c linkedlist.o
	$(CC) -c $(ARGS) linkedlist.c

# making .o file for server_helper
server_helper: server_helper.c
	$(CC) -c $(ARGS) server_helper.c

# making .o file for get_temp
ard: arduino_funcs.c linkedlist
	$(CC) -c $(ARGS) arduino_funcs.c

server: server_helper server.o ard
	$(CC) -o server $(ARGS) server.c server_helper.o arduino_funcs.o linkedlist.o

clear:
	rm -rf *.o server get_temp
