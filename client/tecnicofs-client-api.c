#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define CLIENT_SOCKET_NAME "/tmp/clientsocket"

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

	if (sendto(sockfd, "Hello World", strlen("Hello World")+1, 0, (struct sockaddr *) &server_addr, server_len) < 0) {
		fprintf(stderr,"client: sendto error\n");
		exit(EXIT_FAILURE);
	}

	return 0;
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
