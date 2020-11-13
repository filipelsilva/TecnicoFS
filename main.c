#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int flag_consumer = 1, flag_producer = 1;
int iproducer = 0, iconsumer = 0;

FILE* input; 

/* Syncronization lock */
pthread_mutex_t call_vector;
pthread_cond_t vector_producer, vector_consumer;

/* Filenames for the inputfile and outputfile */
char* outputfile = NULL;
char* inputfile = NULL;

/* Timestamps for the elapsed time */
struct timeval tic, toc;

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

void insertCommand(char* data) {
    call_vector_lock();

    while (numberCommands == MAX_COMMANDS)
        pthread_cond_wait(&vector_producer, &call_vector);

    strcpy(inputCommands[iproducer++], data);

    if (iproducer == MAX_COMMANDS)
        iproducer = 0;

    numberCommands++;

    pthread_cond_signal(&vector_consumer);
    call_vector_unlock();
}


char* removeCommand() {
	char * command;

    /* lock acess to call vector */
    call_vector_lock();

	while (numberCommands == 0) 
		pthread_cond_wait(&vector_consumer, &call_vector);

	command = inputCommands[iconsumer++];

	if (iconsumer == MAX_COMMANDS)
        iconsumer = 0;

    numberCommands--;

	pthread_cond_signal(&vector_producer);
	call_vector_unlock();

	return command;
}


void errorParse() {
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput() {
    char line[MAX_INPUT_SIZE];

	/* Get time after the initialization of the process input */
	gettimeofday(&tic, NULL);

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), input)) {
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }

        switch (token) {
            case 'c':
                if(numTokens != 3)
                    errorParse();
                insertCommand(line);
                return;
            
            case 'l':
                if(numTokens != 2)
                    errorParse();
                insertCommand(line);
                return;
            
            case 'd':
                if(numTokens != 2)
                    errorParse();
                insertCommand(line);
                return;
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
    /* end of file */
    flag_producer = 0;

}


void applyCommands() {
    while (flag_consumer) {
        if (flag_producer == 0 && numberCommands == 0) {
            flag_consumer = 0;

        } else if (numberCommands > 0) {
            const char *command = removeCommand();

            if (command == NULL) {
                continue;
            }

            char token, type;
            char name[MAX_INPUT_SIZE];
            int numTokens = sscanf(command, "%c %s %c", &token, name, &type);

            if (numTokens < 2) {
                fprintf(stderr, "Error: invalid command in Queue\n");
                exit(EXIT_FAILURE);
            }

            int searchResult;
            switch (token) {
                case 'c':
                    switch (type) {
                        case 'f':
                            printf("Create file: %s\n", name);
                            create(name, T_FILE);
                            break;
                        case 'd':
                            printf("Create directory: %s\n", name);
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
                    printf("Delete: %s\n", name);
                    delete(name);
                    break;

                default: { /* error */
                    fprintf(stderr, "Error: command to apply\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void* fnThread_producer() {
    while(flag_consumer)
    processInput();
	return NULL;
}

/* wrapper function, calling applyCommands */
void* fnThread_consumer() {
    applyCommands();
	return NULL;
}

/* process pool initializer and runner */
void processPool() {
	int i;
	pthread_t tid_producer;
	pthread_t tid_consumer[numberThreads - 1];
   	 
	
    if (pthread_create(&tid_producer, NULL, fnThread_producer, NULL)){
        fprintf(stderr, "Error: could not create threads\n");
        exit(EXIT_FAILURE);
    }

	for (i = 0; i < numberThreads - 1; i++) {
        if (pthread_create(&tid_consumer[i], NULL, fnThread_consumer, NULL)) {
            fprintf(stderr, "Error: could not create threads\n");
            exit(EXIT_FAILURE);
        }
    }

    if (pthread_join(tid_producer, NULL)) {
		fprintf(stderr, "Error: could not join thread\n");
	}

	for (i = 0; i < numberThreads - 1; i++) {
		if (pthread_join(tid_consumer[i], NULL)) {
			fprintf(stderr, "Error: could not join thread\n");
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

	if (pthread_mutex_init(&call_vector, NULL)) {
		fprintf(stderr, "Error: could not initialize mutex: call_vector\n");
	}

	if (pthread_cond_init(&vector_producer, NULL)) {
		fprintf(stderr, "Error: could not initialize condition: vector_producer\n");
	}

	if (pthread_cond_init(&vector_consumer, NULL)) {
		fprintf(stderr, "Error: could not initialize condition: vector_consumer\n");
	}

	/* process input */
    input = openFile(inputfile, "r");

  	//sync_locks_init();
	processPool();

	fclose(input);
	//sync_locks_destroy();
	if (pthread_cond_destroy(&vector_consumer)) {
		fprintf(stderr, "Error: could not destroy mutex: vector_consumer\n");
	}

	if (pthread_cond_destroy(&vector_producer)) {
		fprintf(stderr, "Error: could not destroy condition: vector_producer\n");
	}

	if (pthread_mutex_destroy(&call_vector)) {
		fprintf(stderr, "Error: could not destroy condition: call_vector\n");
	}

	print_elapsed_time();

	/* print tree */
	FILE *output = openFile(outputfile, "w");
    print_tecnicofs_tree(output);
	fclose(output);
	
    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
