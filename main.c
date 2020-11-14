#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* Syncronization lock */
pthread_mutex_t call_vector;

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

int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
	if(numberCommands > 0) {
        numberCommands--;
        return inputCommands[headQueue++];  
    }
	return NULL;
}

void errorParse() {
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(FILE *file) {
    char line[MAX_INPUT_SIZE];

	/* Get time after the initialization of the process input */
	gettimeofday(&tic, NULL);

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), file)) {
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
                if(insertCommand(line))
                    break;
                return;
            
            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
			
			case 'm':
				if(numTokens != 3)
					errorParse();
				if(insertCommand(line))
                    break;
                return;

            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
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


void* applyCommands() {
   	while (1) {
		/* lock acess to call vector */
		call_vector_lock();

		if (numberCommands <= 0) {
			call_vector_unlock();
			break;
		}

		const char* command = removeCommand();

		/* unlock acess to call vector */
		call_vector_unlock();

		if (command == NULL) {
			continue;
		}

		char token;
		char name[MAX_INPUT_SIZE], type[MAX_INPUT_SIZE];
		int numTokens = sscanf(command, "%c %s %s", &token, name, type);

		if (numTokens < 2) {
			fprintf(stderr, "Error: invalid command in Queue\n");
			exit(EXIT_FAILURE);
		}

		int searchResult;
		switch (token) {
			case 'c':
				switch (type[0]) {
					case 'f':
						//printf("Create file: %s\n", name);
						create(name, T_FILE);
						break;
					case 'd':
						//printf("Create directory: %s\n", name);
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
				//printf("Delete: %s\n", name);
				delete(name);
				break;
			
			case 'm':
				printf("Moving: %s to %s\n", name, type);
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
	pthread_t tid[numberThreads];
   	 
    for (i = 0; i < numberThreads; i++) {
        if (pthread_create(&tid[i], NULL, applyCommands, NULL)) {
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

	/* process input */
    FILE* input = openFile(inputfile, "r");
	processInput(input);
	fclose(input);
	
	if (pthread_mutex_init(&call_vector, NULL)) {
		fprintf(stderr, "Error: could not initialize mutex: call_vector\n");
	}
  	//sync_locks_init();
	processPool();
	//sync_locks_destroy();
	if (pthread_mutex_destroy(&call_vector)) {
		fprintf(stderr, "Error: could not destroy mutex: call_vector\n");
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
