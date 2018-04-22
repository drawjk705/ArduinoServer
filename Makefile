CC = clang
ARGS = -Wall -g -lpthread

all: server_helper ard times json server

# making .o file for times
times: times.c
	$(CC) -c $(ARGS) times.c

# making .o file for json
json: json.c
	$(CC) -c $(ARGS) json.c

# making .o file for server_helper
server_helper: server_helper.c
	$(CC) -c $(ARGS) server_helper.c

# making .o file for arduino functions
ard: arduino_funcs.c json times
	$(CC) -c $(ARGS) arduino_funcs.c json.o times.o

server: times json server_helper ard server.o
	$(CC) -o server $(ARGS) server.c server_helper.o arduino_funcs.o json.o times.o

clear:
	rm -rf *.o server get_temp
