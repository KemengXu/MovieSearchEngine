#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>


#include "QueryProtocol.h"
#include "MovieSet.h"
#include "MovieIndex.h"
#include "DocIdMap.h"
#include "Hashtable.h"
#include "QueryProcessor.h"
#include "FileParser.h"
#include "DirectoryParser.h"
#include "FileCrawler.h"
#include "Util.h"

#define BUFFER_SIZE 1000

int Cleanup();

DocIdMap docs;
MovieTitleIndex docIndex;

#define SEARCH_RESULT_LENGTH 5000

char movieSearchResult[SEARCH_RESULT_LENGTH];
char gb[20] = "GOODBYE";

void sigchld_handler(int s) {
  write(0, "Handling zombies...\n", 20);
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}


void sigint_handler(int sig) {
  write(0, "Ahhh! SIGINT!\n", 14);
  Cleanup();
  exit(0);
}

int RunQuery(char* term) {
  int numResult = 0;
  SearchResultIter results = FindMovies(docIndex, term);
  if (results == NULL) {
    printf("No results for this term. Please try another.\n");
    return numResult;
  } else {
    SearchResult sr = (SearchResult)malloc(sizeof(*sr));
    if (sr == NULL) {
      printf("Couldn't malloc SearchResult in main.c\n");
      return -1;
    }
    int result;
    int i = 0;
    SearchResultGet(results, sr);
    numResult++;
    CopyRowFromFile(sr, docs, &movieSearchResult[i]);
    movieSearchResult[strlen(movieSearchResult)] = '\n';
    // Check if there are more
    i = strlen(movieSearchResult);
    while (SearchResultIterHasMore(results) != 0) {
      result =  SearchResultNext(results);
      if (result < 0) {
        printf("error retrieving result\n");
        break;
      }
      if (i > SEARCH_RESULT_LENGTH - 2 * BUFFER_SIZE) {
        printf("too many result, please narrow down your keyword\n");
        break;
      }
      numResult++;
      SearchResultGet(results, sr);
      CopyRowFromFile(sr, docs, &movieSearchResult[i]);
      movieSearchResult[strlen(movieSearchResult)] = '\n';
      i = strlen(movieSearchResult);
      // printf("Original::::::: %s\n", movieSearchResult);
    }
    free(sr);
    DestroySearchResultIter(results);
    return numResult;
  }
}

/**
 * Return 0 for successful connection;
 * Return -1 for some error.
 */
int HandleClient(int client_fd, char* query) {
  // Run query and get responses
  int numResult = RunQuery(query);
  // Send number of responses
  if (numResult == 0) {
    SendGoodbye(client_fd);
    close(client_fd);
    return 0;
  }
  int length = snprintf(NULL, 0, "%d", numResult);
  char* str = (char*)malloc(sizeof(char) * (length + 1));
  snprintf(str, length + 1, "%d", numResult);
  write(client_fd, str, strlen(str));
  free(str);
  // Wait for ACK
  char buffer3[BUFFER_SIZE];
  int len = read(client_fd, buffer3, BUFFER_SIZE - 1);
  buffer3[len] = '\0';
  CheckAck(buffer3);
  // For each response
  write(client_fd, movieSearchResult, strlen(movieSearchResult));
    // Send response
    // Wait for ACK
  char buffer4[BUFFER_SIZE];
  len = read(client_fd, buffer4, BUFFER_SIZE - 1);
  buffer4[len] = '\0';
  CheckAck(buffer4);
  // Cleanup
  memset(movieSearchResult, 0, strlen(movieSearchResult));
  memset(buffer3, 0, strlen(buffer3));
  memset(buffer4, 0, strlen(buffer4));
  // Send GOODBYE
  SendGoodbye(client_fd);
  // close connection.
  close(client_fd);
  return 0;
}

/**
 *
 */
int HandleConnections(int sock_fd, int debug) {
  // Step 5: Accept connection
  // Fork on every connection
  if (debug == 1) {
    sleep(10);
  }
  printf("Waiting for connection...\n");
  int client_fd = accept(sock_fd, NULL, NULL);
  printf("Connection made: client_fd=%d\n", client_fd);
  char buffer[BUFFER_SIZE];
  int len = read(client_fd, buffer, sizeof(buffer) - 1);
  buffer[len] = '\0';
  printf("Read %d chars\n", len);
  printf("===\n");
  printf("%s\n", buffer);
  // Send ACK
  SendAck(client_fd);
  // Listen for query
  char buffer1[BUFFER_SIZE];
  len = read(client_fd, buffer1, BUFFER_SIZE - 1);
  buffer1[len] = '\0';
  // If query is GOODBYE close ocnnection
  if (strcmp(buffer1, gb) == 0) {
    close(client_fd);
    printf("GOODBYE!!!\n\n");
    memset(buffer, 0, strlen(buffer));
    memset(buffer1, 0, strlen(buffer1));
    return 0;
  }
  printf("Got query: %s\n", buffer1);
  HandleClient(client_fd, buffer1);
  memset(buffer, 0, strlen(buffer));
  memset(buffer1, 0, strlen(buffer1));
  return 0;
}

int Setup(char *dir) {
  struct sigaction sa;

  sa.sa_handler = sigchld_handler;  // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  struct sigaction kill;

  kill.sa_handler = sigint_handler;
  kill.sa_flags = 0;  // or SA_RESTART
  sigemptyset(&kill.sa_mask);

  if (sigaction(SIGINT, &kill, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("Crawling directory tree starting at: %s\n", dir);
  // Create a DocIdMap
  docs = CreateDocIdMap();
  CrawlFilesToMap(dir, docs);
  printf("Crawled %d files.\n", NumElemsInHashtable(docs));

  // Create the index
  docIndex = CreateMovieTitleIndex();

  if (NumDocsInMap(docs) < 1) {
    printf("No documents found.\n");
    return 0;
  }

  // Index the files
  printf("Parsing and indexing files...\n");
  ParseTheFiles(docs, docIndex);
  printf("%d entries in the index.\n", NumElemsInHashtable(docIndex->ht));
  return NumElemsInHashtable(docIndex->ht);
}

int Cleanup() {
  DestroyMovieTitleIndex(docIndex);
  DestroyDocIdMap(docs);
  return 0;
}

int main(int argc, char **argv) {
  // port
  char *port = NULL;
  char *dir_to_crawl = NULL;

  int debug_flag = 0;
  int c;

  while ((c = getopt(argc, argv, "dp:f:")) != -1) {
    switch (c) {
      case 'd':
        debug_flag = 1;
        break;
      case 'p':
        port = optarg;
        break;
      case 'f':
        dir_to_crawl = optarg;
        break;
      }
  }

  if (port == NULL) {
    printf("No port provided; please include with a -p flag.\n");
    exit(0);
  }

  if (dir_to_crawl == NULL) {
    printf("No directory provided; please include with a -f flag.\n");
    exit(0);
  }

  int num_entries = Setup(dir_to_crawl);
  if (num_entries == 0) {
    printf("No entries in index. Quitting. \n");
    exit(0);
  }

  int s;

  // Step 1: Get address stuff
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }
  // Step 2: Open socket
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  // Step 3: Bind socket
  if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
    perror("bind()");
    exit(1);
  }
  // Step 4: Listen on the socket
  if (listen(sock_fd, 10) != 0) {
    perror("listen()");
    exit(1);
  }
  struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
  printf("Listening on file descriptor %d, ", sock_fd);
  printf("port %d\n", ntohs(result_addr->sin_port));
  freeaddrinfo(result);
  while (1) {
    HandleConnections(sock_fd, debug_flag);
  }
  // Got Kill signal
  close(sock_fd);
  Cleanup();
  return 0;
}
