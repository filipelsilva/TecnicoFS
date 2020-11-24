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
#define SOCKET_NAME "/tmp/servidorsocket"
#define INDIM 30
#define OUTDIM 512

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* Syncronization lock */
pthread_mutex_t call_vector;

/* Conditional syncronization */
pthread_cond_t producer, consumer;
int prod_num = 0, cons_num = 0;
int flag = 1;

/* Inputfile */
FILE* input; 

/* Filenames for the inputfile and outputfile */
char* outputfile = NULL;
char* inputfile = NULL;

/* Timestamps for the elapsed time */
struct timeval tic, toc;

void argumentParser(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Error: invalid arguments\n");
		exit(EXIT_FAILURE);
	}

	inputfile = argv[1];
	outputfile = argv[2];
	numberThreads = atoi(argv[3]);

	if (numberThreads < 1) {
		fprintf(stderr, "Error: invalid number of threads\n");
		exit(EXIT_FAILURE);
	}
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

/* Initializes locks */
void sync_locks_init() {
	if (pthread_mutex_init(&call_vector, NULL)) {
		fprintf(stderr, "Error: could not initialize mutex: call_vector\n");
	}

	if (pthread_cond_init(&producer, NULL)) {
		fprintf(stderr, "Error: could not initialize condition: producer\n");
	}

	if (pthread_cond_init(&consumer, NULL)) {
		fprintf(stderr, "Error: could not initialize condition: consumer\n");
	}
}

/* Destroys locks */
void sync_locks_destroy() {
	if (pthread_cond_destroy(&consumer)) {
		fprintf(stderr, "Error: could not destroy mutex: consumer\n");
	}

	if (pthread_cond_destroy(&producer)) {
		fprintf(stderr, "Error: could not destroy condition: producer\n");
	}

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

/* Activates the conditional wait */
void cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
	if (pthread_cond_wait(cond, mutex)) {
		fprintf(stderr, "Error: could not block on condition variable\n");
	}
}

/* Signals the conditional wait to move on */
void signal(pthread_cond_t* cond) {
	if (pthread_cond_signal(cond)) {
		fprintf(stderr, "Error: could not signal the condition variable\n");
	}
}

void insertCommand(char* data) {
	call_vector_lock();

	while (numberCommands == MAX_COMMANDS) {
		cond_wait(&producer, &call_vector);
	}

	strcpy(inputCommands[prod_num % MAX_COMMANDS], data);
	prod_num++;
	numberCommands++;

	signal(&consumer);
	call_vector_unlock();
}

char* removeCommand() {
	char* command;
	call_vector_lock();

	while (numberCommands == 0) {
		if (flag) {
			cond_wait(&consumer, &call_vector);
		} else {
			call_vector_unlock();
			return NULL;
		}
	}

	command = inputCommands[cons_num % MAX_COMMANDS];
	cons_num++;
	numberCommands--;

	signal(&producer);
	call_vector_unlock();

	return command;
}

void errorParse() {
	fprintf(stderr, "Error: command invalid\n");
	exit(EXIT_FAILURE);
}

void* processInput() {
	char line[MAX_INPUT_SIZE];
	
	/* Get time after the initialization of the process input */
	gettimeofday(&tic, NULL);

	/* break loop with ^Z or ^D */
	while (flag) {
		if (fgets(line, sizeof(line)/sizeof(char), input)) {
			char token;
			char name[MAX_INPUT_SIZE], type[MAX_INPUT_SIZE];
			int numTokens = sscanf(line, "%c %s %s", &token, name, type);

			/* perform minimal validation */
			if (numTokens < 1) {
				continue;
			}
			switch (token) {
				case 'c':
					if(numTokens != 3)
						errorParse();
					insertCommand(line);
					break;

				case 'l':
					if(numTokens != 2)
						errorParse();
					insertCommand(line);
					break;

				case 'd':
					if(numTokens != 2)
						errorParse();
					insertCommand(line);
					break;

				case 'm':
					if(numTokens != 3)
						errorParse();
					insertCommand(line);
					break;

				case '#':
					break;

				default: { /* error */
							 errorParse();
						 }
			}
		} else {
			flag = 0;
		}
	}
	return NULL;
}

void* applyCommands() {
	while (1) {
		const char* command = removeCommand();

		if (command == NULL) {
			return NULL;
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
	return NULL;
}

/* process pool initializer and runner */
void processPool() {
	int i;
	pthread_t tid_producer, tid[numberThreads]; //OU numberThreads - 1????

	if (pthread_create(&tid_producer, NULL, processInput, NULL)){
		fprintf(stderr, "Error: could not create threads\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < numberThreads; i++) {
		if (pthread_create(&tid[i], NULL, applyCommands, NULL)) {
			fprintf(stderr, "Error: could not create threads\n");
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_join(tid_producer, NULL)) {
		fprintf(stderr, "Error: could not join thread\n");
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

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

    if (addr == NULL)
        return 0;

    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

int main(int argc, char* argv[]) {
    int sockfd;
    struct sockaddr_un server_addr;
    socklen_t addrlen;

    // Criar socket
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "server: can't open socket\n");
        exit(EXIT_FAILURE);
    }

    addrlen = setSockAddrUn (SOCKET_NAME, &server_addr);

    if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
        fprintf(stderr, "server: bind error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        struct sockaddr_un client_addr;
        char in_buffer[INDIM];
        int c;

        addrlen = sizeof(struct sockaddr_un);

        c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
                     (struct sockaddr *)&client_addr, &addrlen);

        printf("%s\n", in_buffer);

        if (c <= 0) continue;

        in_buffer[c]='\0';
    }

	/* init filesystem */
	init_fs();

	argumentParser(argc, argv);

	sync_locks_init();

	input = openFile(inputfile, "r");
	processPool();
	fclose(input);

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
