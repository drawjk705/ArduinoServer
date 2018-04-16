/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include "server_helper.h"
#include "arduino_funcs.h"

char quit;

void* say_hello(void* p) {
  int i = 0;
  while(quit != 'q') {
    // pthread_mutex_lock(&lock);
    printf("%d\n", i++);
    // sleep(2);
    if (i % 5 == 0) {
      // pthread_mutex_unlock(&lock);
    }
  }
  pthread_exit(NULL);
}

int start_server(int PORT_NUMBER) {
  
    /*************************************************************************/
    /********************* Getting the server set up *************************/
    /*************************************************************************/

    // structs to represent the server and client
    struct sockaddr_in server_addr, client_addr;    
      
    int sock; // socket descriptor

    // 1. socket: creates a socket descriptor that you later use to make other system calls
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Socket");
      exit(1);
    }
    int temp;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) == -1) {
      perror("Setsockopt");
      exit(1);
    }

    // configure the server
    server_addr.sin_port = htons(PORT_NUMBER); // specify port number
    server_addr.sin_family = AF_INET;         
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero), 8); 
    
    // 2. bind: use the socket and associate it with the port number
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
      perror("Unable to bind");
      exit(1);
    }

    // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
    if (listen(sock, 1) == -1) {
      perror("Listen");
      exit(1);
    }
        
    // once you get here, the server is set up and about to start listening
    printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
    fflush(stdout);
   

    // 4. accept: wait here until we get a connection on that port
    int sin_size = sizeof(struct sockaddr_in);

    /*************************************************************************/
    /*************************************************************************/

    /*************************************************************************/
    /***************** Open Arduino Connection to Server *********************/
    /*************************************************************************/



    // name of Serial Port
    char* filename = "/dev/ttyACM0";
    // char* filename = "/dev/ttyACM1";

    // set quit flag to '\0'
    quit = '\0';

    // retrieve file descriptor from Arduino
    int ard_fd = get_started(filename);

    // write intial null char to Arduino to clear out
    write_to_arduino(ard_fd, '\0');

    // create a packet with relevant data

    packet* pack0 = malloc(sizeof(packet));
    pack0->filename = filename;
    pack0->ard_fd = ard_fd;
    pack0->quit_ptr = &quit;

    pthread_t ard_t;

    pthread_create(&ard_t, NULL, &get_temps, pack0);

    // keep on accepting requests as long as
    // haven't received command to close server
    while (quit != 'q') {

      // >>>>> // send appropriate messages to site <<<<<< //
      if (check_if_open(ard_fd) == -1) {
        printf("Arduino has been disconnected\n");
      } else {
        printf("Arduino is connected\n");
      }

      /***********************************/
          // create close_server thread //
          pthread_t close;
          pthread_create(&close, NULL, &close_server, NULL);

      /***********************************/

      /**************************************************/
      /** set up select() for accept()ing HTML requests */
      int sret;                     // to get return value from select()
      fd_set readfds;               // bit array to represent fds
      struct timeval timeout;       // struct to determine how long to wait before timeout
      FD_ZERO(&readfds);            // zero out bit array
      FD_SET(sock, &readfds);       // set bit array
      timeout.tv_sec = 3;           // set timeout time
      timeout.tv_usec = 0;
      /**************************************************/

      sret = select(8, &readfds, NULL, NULL, &timeout);     // run the select()

      // if have received an HTTP request...
      if (sret != 0) {
        
        // accept
        int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        if (fd != -1) {

          pack0->client_addr = client_addr; // add additional info to packet
          pack0->fd = fd;
          
          pthread_t req;                    // create request thread
          pthread_create(&req, NULL, &handle_connection, pack0);
          
          pthread_join(req, NULL);          // join request thread
        }
      }
      pthread_join(close, NULL);            // join close thread
    }

    pthread_join(ard_t, NULL);              // join thread that handles Arduino behavior

    // 8. close: close the socket
    close(sock);
    printf("Server shutting down\n");

    return 0;
}


/**
 * function for thread to wait for user
 * input to close the server
 * @param p [description]
 */
void* close_server(void* p) {

    quit = '\0';            // intialize quit

    /** preparing the select() for stdin */

    int fd = 0;             // create file descriptor

    int sret;               // to get return value from select()
    
    fd_set readfds;         // bit array to represent fds

    struct timeval timeout; // struct to determine how long to wait before timeout

    FD_ZERO(&readfds);      // zero out bit array

    FD_SET(fd, &readfds);   // set bit array

    timeout.tv_sec = 1;     // set timeout time
    timeout.tv_usec = 0;

    // run the select
    sret = select(8, &readfds, NULL, NULL, &timeout);
    if (sret != 0) {
      printf("pressed q\n");
        quit = getchar();
    }

    // exit
    pthread_exit(NULL);
}

/**
 * handle connection when request comes in
 * @param p packet containing relevant data
 */
void* handle_connection(void* p) {

    // unpack packet
    packet* pack = (packet*) p;
    struct sockaddr_in client_addr = pack->client_addr;
    int fd = pack->fd;
    int ard_fd = pack->ard_fd;

    printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // buffer to read data into
    char request[1024];
    
    // 5. recv: read incoming message (request) into buffer
    int bytes_received = recv(fd,request,1024,0);
    // null-terminate the string
    request[bytes_received] = '\0';
    // print it to standard out
    printf("This is the incoming request:\n%s\n", request);

    // this is the message that we'll send back
    char* reply = malloc(sizeof(char) * 100);
    strcpy(reply, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n");      // <<<<<<< specify if is text/css or jscript

    // parse request
    char* req = get_path(request);

    // ignore favicon.ico requests
    if (strcmp(req, "favicon.ico") == 0) {
      close(fd);
      free(req);
      pthread_exit(NULL);
    }

    // read the HTML file, and append it to the reply
    char* add = read_html_file(req);
    free(req);

    // realloc() reply
    reply = (char*) realloc(reply, sizeof(char) * ((strlen(reply) + strlen(add) + 1)));
    // concatenate
    strcat(reply, add);
    free(add);

    // 6. send: send the outgoing message (response) over the socket
    // note that the second argument is a char*, and the third is the number of chars   
    send(fd, reply, strlen(reply), 0);
    
    // handle if it's a POST request
    if (is_get(request) == 1) {
      char* post = get_post(request);
      char c = parse_post(post);
      printf("\n\n%c\n\n\n", c);
      if (c == 'r') {
        printf("Making it red\n");
        write_to_arduino(ard_fd, 'r');
      } 
      if (c == 'g') {
        printf("making it green\n");
        write_to_arduino(ard_fd, 'g');
      }
      if (c == 'b') {
        printf("making it blue\n");
        write_to_arduino(ard_fd, 'b');
      }
    }
    
    free(reply);

    // 7. close: close the connection
    close(fd);
    printf("Server closed connection\n");
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

  // check the number of arguments
  if (argc != 2) {
      printf("\nUsage: %s [port_number]\n", argv[0]);
      exit(-1);
  }

  int port_number = atoi(argv[1]);
  if (port_number <= 1024) {
    printf("\nPlease specify a port number greater than 1024\n");
    exit(-1);
  }
  

  start_server(port_number);
}
