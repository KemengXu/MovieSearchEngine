#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "QueryProtocol.h"

char *port_string = "1500";
unsigned short int port;
char *ip = "127.0.0.1";

#define BUFFER_SIZE 1000

#define SEARCH_RESULT_LENGTH 5000

void RunQuery(char *query) {
  // Find the address
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* IPv4 only */
  hints.ai_socktype = SOCK_STREAM; /* TCP */

  int s;
  s = getaddrinfo(ip, port_string, &hints, &result);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }
  if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
    perror("connect");
    exit(2);
  } else {
    printf("Connection is good!\n");
  }
  freeaddrinfo(result);
  // Do the query-protocol
  char *buffer = "Howdy there";
  printf("SENDING: %s", buffer);
  printf("===\n");
  write(socket_fd, buffer, strlen(buffer));
  // memset(buffer,0,strlen(buffer));
  char resp[BUFFER_SIZE];
  int len = read(socket_fd, resp, BUFFER_SIZE-1);
  resp[len] = '\0';
  if (CheckAck(resp) != 0) {
    printf("ERROR! responce from server is not ACK\n");
    exit(3);
  }
  memset(resp, 0, strlen(resp));
  printf("SENDING: %s", query);
  printf("===\n");
  write(socket_fd, query, strlen(query));
  char resp1[BUFFER_SIZE];
  printf("searching...\n");
  len = read(socket_fd, resp1, BUFFER_SIZE-1);
  resp1[len] = '\0';
  // printf("%d++++++++++++\n", strcmp(resp1, "GOODBYE\0"));
  if (strcmp(resp1, "GOODBYE\0") == 0) {
    close(socket_fd);
    memset(resp1, 0, strlen(resp1));
    printf("Fount 0 result\nGOODBYE!!!\n\n");
    return;
  }
  printf("Found %s results...\n", resp1);
  SendAck(socket_fd);
  memset(resp1, 0, strlen(resp1));
  char resp2[SEARCH_RESULT_LENGTH];
  len = read(socket_fd, resp2, SEARCH_RESULT_LENGTH-1);
  resp2[len] = '\0';
  printf("%s\n", resp2);
  SendAck(socket_fd);
  memset(resp2, 0, strlen(resp2));
  close(socket_fd);
  printf("GOODBYE!!!\n\n");
}

void RunPrompt() {
  char input[BUFFER_SIZE];

  while (1) {
    printf("Enter a term to search for, or q to quit: ");
    scanf("%s", input);

    printf("input was: %s\n", input);

    if (strlen(input) == 1) {
      if (input[0] == 'q') {
        printf("Thanks for playing! \n");
        return;
      }
    }
    printf("\n\n");
    RunQuery(input);
  }
}

// This function connects to the given IP/port to ensure
// that it is up and running, before accepting queries from users.
// Returns 0 if can't connect; 1 if can.
int CheckIpAddress(char *ip, char *port) {
  // Connect to the server
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* IPv4 only */
  hints.ai_socktype = SOCK_STREAM; /* TCP */

  int s;
  s = getaddrinfo(ip, port, &hints, &result);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }
  if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
    perror("connect");
    exit(2);
  } else {
    printf("Connection is good!\n");
  }

  // Listen for an ACK
  char *buffer = "Howdy there";
  printf("SENDING: %s", buffer);
  printf("===\n");
  write(socket_fd, buffer, strlen(buffer));

  char resp[BUFFER_SIZE];
  int len = read(socket_fd, resp, BUFFER_SIZE-1);
  resp[len] = '\0';
  if (CheckAck(resp) != 0) {
    printf("ERROR! responce from server is not ACK\n");
    exit(3);
  }
  // Send a goodbye
  if (SendGoodbye(socket_fd) != 0) {
    printf("ERROR! sending goodbye to server failed\n");
    exit(4);
  }
  // Close the connection
  close(socket_fd);
  // memset(buffer,0,strlen(buffer));
  freeaddrinfo(result);
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Incorrect number of arguments. \n");
    printf("Correct usage: ./queryclient [IP] [port]\n");
  } else {
    ip = argv[1];
    port_string = argv[2];
  }
  if (CheckIpAddress(ip, port_string)) {
    printf("Waiting for client input \n");
    RunPrompt();
  }
  return 0;
}
