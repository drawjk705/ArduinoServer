/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

char resp[100];
int quit;


pthread_mutex_t lock;

typedef struct Packet packet;
struct Packet
{
    struct sockaddr_in client_addr;
    int fd;
};

void* handle_connection(void* p);
char* parse_request(char* request);
char* read_html_file(char* filename);
void* exit_server(void* p);


// extern char* get_temp(char* file_name);

int start_server(int PORT_NUMBER)
{

      // create lock
      if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("couldn't init lock");
        return -1;
      }

      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;    
      
      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUMBER); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 
      
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

      while (1) {
        int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        if (fd != -1) {

            packet* p = malloc(sizeof(packet));
            p->client_addr = client_addr;
            p->fd = fd;
            pthread_t t1;
            pthread_create(&t1, NULL, &handle_connection, p);
            pthread_join(t1, NULL);
            free(p);
        }
      }

      // 8. close: close the socket
      close(sock);
      printf("Server shutting down\n");
  
      return 0;
}

// void* exit_server(void* p) {
    
//     char str[10];
//     sleep(10);
//     scanf("%s", str);

//     if (str[0] == 'q') {
//         quit = 1;
//     } else {
//         quit = 0;
//     }
//     pthread_exit(&quit);
// }

void* handle_connection(void* p) {

    // unpack packet
    packet* pack = (packet*) p;
    struct sockaddr_in client_addr = pack->client_addr;
    int fd = pack->fd;

    printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

    // buffer to read data into
    char request[1024];
    
    // 5. recv: read incoming message (request) into buffer
    int bytes_received = recv(fd,request,1024,0);
    // null-terminate the string
    request[bytes_received] = '\0';
    // print it to standard out
    printf("This is the incoming request:\n%s\n", request);

    // parse request
    char* req = parse_request(request);
    if (strcmp(req, "favicon.ico") == 0) {
        close(fd);
        pthread_exit(NULL);
    }

    // this is the message that we'll send back
    char reply[9999] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"; //<html><p>"; //</p></html>";
    char* add = read_html_file(req);
    printf("%s\n", add);
    strcat(reply, add);
      // strcat(reply, resp);
      // char* end = "</p></html>";
      // strcat(reply, end);
      // printf("%s\n", resp);
      printf("REPLY: %s\n", reply);

    // 6. send: send the outgoing message (response) over the socket
    // note that the second argument is a char*, and the third is the number of chars   
    send(fd, reply, strlen(reply), 0);
    
    // 7. close: close the connection
    close(fd);
    printf("Server closed connection\n");
    pthread_exit(NULL);
}

char* parse_request(char* request) {
    
    printf("\n PARSING REQUEST: %s\n", request);

    int start = 0, end = 0;
    for (int i = 0; i < strlen(request); i++) {
        if (request[i] == '/' && start == 0) {
            start = i + 1;
        }
        if (request[i] == ' ' && start > 0) {
            end = i;
            break;
        }
    }

    char* out = malloc(sizeof(char) * (end - start + 1));

    for (int i = 0; i < end - start; i++) {
        out[i] = request[i + start];
    }
    // null terminate
    out[end - start] = '\0';
    return out;
}

char* read_html_file(char* filename) {
    FILE* fp = fopen(filename, "r");

    // get length of file
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);

    // malloc
    char* out = malloc(sizeof(char) * (len + 1));
    if (out == NULL) {
        return NULL;
    }

    // rewind fp to start
    rewind(fp);

    // read file into out
    fread(out, sizeof(char), len - 1, fp);

    // null terminate
    out[len] = '\0';

    // close file, and return
    fclose(fp);
    return out;
}



int main(int argc, char *argv[])
{

    // strcpy(resp, get_temp("/dev/ttyACM0"));
    // printf("%s\n", resp);


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

