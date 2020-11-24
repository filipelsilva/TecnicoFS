#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define CLIENT_SOCKET_NAME "/tmp/clientesocket"

char * server_path;
int sockfd;

int setSockAddrUn(char *path, struct sockaddr_un *addr) {
  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

int tfsCreate(char *filename, char nodeType) {
  socklen_t server_len;
  struct sockaddr_un server_addr;
  server_len = setSockAddrUn(server_path, &server_addr);
  char str[MAX_INPUT_SIZE];
  int answer;

  sprintf(str, "c %s %c", filename, nodeType);

  if (sendto(sockfd, str, strlen(str)+1, 0, (struct sockaddr *) &server_addr, server_len) < 0) {
    fprintf(stderr,"client: sendto error\n");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &answer, sizeof(int), 0, 0, 0) < 0) {
    fprintf(stderr,"client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return answer;
}

int tfsDelete(char *path) {
  socklen_t server_len;
  struct sockaddr_un server_addr;
  server_len = setSockAddrUn(server_path, &server_addr);
  char str[MAX_INPUT_SIZE];
  int answer;

  sprintf(str, "d %s", path);

  if (sendto(sockfd, str, strlen(str)+1, 0, (struct sockaddr *) &server_addr, server_len) < 0) {
    fprintf(stderr,"client: sendto error\n");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &answer, sizeof(int), 0, 0, 0) < 0) {
    fprintf(stderr,"client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return answer;
}

int tfsMove(char *from, char *to) {
  socklen_t server_len;
  struct sockaddr_un server_addr;
  server_len = setSockAddrUn(server_path, &server_addr);
  char str[MAX_INPUT_SIZE];
  int answer;

  sprintf(str, "m %s %s", from, to);

  if (sendto(sockfd, str, strlen(str)+1, 0, (struct sockaddr *) &server_addr, server_len) < 0) {
    fprintf(stderr,"client: sendto error\n");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &answer, sizeof(int), 0, 0, 0) < 0) {
    fprintf(stderr,"client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return answer;
}

int tfsLookup(char *path) {
  socklen_t server_len;
  struct sockaddr_un server_addr;
  server_len = setSockAddrUn(server_path, &server_addr);
  char str[MAX_INPUT_SIZE];
  int answer;

  sprintf(str, "l %s", path);

  if (sendto(sockfd, str, strlen(str)+1, 0, (struct sockaddr *) &server_addr, server_len) < 0) {
    fprintf(stderr,"client: sendto error\n");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &answer, sizeof(int), 0, 0, 0) < 0) {
    fprintf(stderr,"client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return answer;
}

int tfsMount(char * sockPath) {
  socklen_t client_len;
  struct sockaddr_un client_addr;
  server_path = malloc((strlen(sockPath)+1));

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr,"client: can't open socket\n");
    exit(EXIT_FAILURE);
  }

  unlink(CLIENT_SOCKET_NAME);
  client_len = setSockAddrUn (CLIENT_SOCKET_NAME, &client_addr);

  if (bind(sockfd, (struct sockaddr *) &client_addr, client_len) < 0) {
    fprintf(stderr,"client: bind error\n");
    exit(EXIT_FAILURE);
  }

  strcpy(server_path, sockPath);

  return 0;
}

int tfsUnmount() {
  return -1;
}
