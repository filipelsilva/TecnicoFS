#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define CLIENT_SOCKET_NAME "cliente"

char * server_path;

int tfsCreate(char *filename, char nodeType) {
  return -1;
}

int tfsDelete(char *path) {
  return -1;
}

int tfsMove(char *from, char *to) {
  return -1;
}

int tfsLookup(char *path) {
  return -1;
}

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

    if (addr == NULL)
        return 0;

    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

int tfsMount(char * sockPath) {
  int sockfd;
  socklen_t /*server_len,*/ client_len;
  struct sockaddr_un /*server_addr,*/ client_addr;
  //char buffer[1024];

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) <= 0){
    perror("client: can't open socket\n");
    exit(EXIT_FAILURE);
  }

  client_len = setSockAddrUn(CLIENT_SOCKET_NAME, &client_addr);

  if(bind(sockfd, (struct sockaddr *) &client_addr, client_len) < 0){
    fprintf(stderr, "client: bind error\n");
    exit(EXIT_FAILURE);
  }

  strcpy(server_path, sockPath);
  //server_len = setSockAddrUn(server_path, &server_addr);

  return -1;
}

int tfsUnmount() {
  return -1;
}
