#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define INDIM 30
#define OUTDIM 512

int numberThreads = 0;
char * serverName;
int sockfd;

/* Syncronization lock */
pthread_mutex_t call_vector;

/* Filenames for the inputfile and outputfile */
char* outputfile = NULL;

/* Timestamps for the elapsed time */
struct timeval tic, toc;

/* Initializes locks */
void sync_locks_init() {
  if (pthread_mutex_init(&call_vector, NULL)) {
    fprintf(stderr, "Error: could not initialize mutex: call_vector\n");
  }
}

/* Destroys locks */
void sync_locks_destroy() {
  if (pthread_mutex_destroy(&call_vector)) {
    fprintf(stderr, "Error: could not destroy mutex: call_vector\n");
  }
}

/* Call vector -> syncronization lock enabler */
void call_vector_lock() {
  if (pthread_mutex_lock(&call_vector)) {
    fprintf(stderr, "Error: could not lock mutex: call_vector\n");
  }
}


/* Call vector -> syncronization lock disabler */
void call_vector_unlock() {
  if (pthread_mutex_unlock(&call_vector)) {
    fprintf(stderr, "Error: could not unlock mutex: call_vector\n");
  }
}

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

void argumentParser(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Error: invalid arguments\n");
		exit(EXIT_FAILURE);
	}

	numberThreads = atoi(argv[1]);
	serverName = argv[2];

	if (numberThreads < 1) {
		fprintf(stderr, "Error: invalid number of threads\n");
		exit(EXIT_FAILURE);
	}
}

int fsMount(){
  struct sockaddr_un server_addr;
  socklen_t addrlen;

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "server: can't open socket\n");
    exit(EXIT_FAILURE);
  }

  addrlen = setSockAddrUn (serverName, &server_addr);

  if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
    fprintf(stderr, "server: bind error\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}

/* File opening with NULL checker */
FILE* openFile(char* name, char* mode) {
  FILE* fp = fopen(name, mode);

  if (fp == NULL) {
    fprintf(stderr, "Error: could not open file\n");
    exit(TECNICOFS_ERROR_FILE_NOT_FOUND);
  }

  return fp;
}

void applyCommands(const char* command) {
  printf("%s", command);
  if (command == NULL) {
    return ;
  }

  call_vector_lock();
  char token;
  char name[MAX_INPUT_SIZE], type[MAX_INPUT_SIZE];
  int numTokens = sscanf(command, "%c %s %s", &token, name, type);
  call_vector_unlock();

  if (numTokens < 2) {
    fprintf(stderr, "Error: invalid command in Queue\n");
    exit(EXIT_FAILURE);
  }

  int searchResult;
  switch (token) {
    case 'c':
      switch (type[0]) {
        case 'f':
          create(name, T_FILE);
          break;
        case 'd':
          create(name, T_DIRECTORY);
          break;
        default:
          fprintf(stderr, "Error: invalid node type\n");
          exit(EXIT_FAILURE);
      }
      break;

    case 'l':
      searchResult = lookup(name);
      if (searchResult >= 0)
        printf("Search: %s found\n", name);
      else
        printf("Search: %s not found\n", name);

      break;

    case 'd':
      delete(name);
      break;

    case 'm':
      move(name, type);
      break;

    default: { /* error */
           fprintf(stderr, "Error: command to apply\n");
           exit(EXIT_FAILURE);
         }
  }
}

void *receiveCommands(){
  while(1){
    struct sockaddr_un client_addr;
    socklen_t addrlen;
    char in_buffer[INDIM];
    int c;

    addrlen = sizeof(struct sockaddr_un);

    c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
                 (struct sockaddr *)&client_addr, &addrlen);

    if (c <= 0) continue;

    in_buffer[c]='\0';

    applyCommands(in_buffer);

    printf("%s\n", in_buffer);

  }

  return NULL;
}

/* process pool initializer and runner */
void processPool() {
	int i;
	pthread_t tid[numberThreads]; //OU numberThreads - 1????

	for (i = 0; i < numberThreads; i++) {
		if (pthread_create(&tid[i], NULL, receiveCommands, NULL)) {
			fprintf(stderr, "Error: could not create threads\n");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < numberThreads; i++) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "Error: could not join thread\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Get time after all has been done */
	gettimeofday(&toc, NULL);
}

void print_elapsed_time() {
	printf("TecnicoFS completed in %.4f seconds.\n",\
			(double) (toc.tv_usec - tic.tv_usec) / \
			1000000 + (double) (toc.tv_sec - tic.tv_sec));
}

int main(int argc, char* argv[]) {
  /* init filesystem */
  init_fs();

  argumentParser(argc, argv);

  /* Create server socket*/
  fsMount();

  // /* Get time after the initialization of the process input */
  //	gettimeofday(&tic, NULL);
  sync_locks_init();

	processPool();

	sync_locks_destroy();

	print_elapsed_time();

	/* print tree */
	FILE *output = openFile(outputfile, "w");
	print_tecnicofs_tree(output);
	fclose(output);

	/* release allocated memory */
	destroy_fs();
	exit(EXIT_SUCCESS);
}
