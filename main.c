#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h> // CONFIRMAR SE PODEMOS USAR ISTO
#include <pthread.h>
#include <unistd.h>
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* Syncronization locks */
pthread_mutex_t call_vector;
pthread_mutex_t mutex;
pthread_rwlock_t rwlock;

/* Filenames for the inputfile and outputfile */
char* outputfile = NULL;
char* inputfile = NULL;

/* Syncronization strategy */
char* syncStrategy = NULL;

/* Timestamps for the elapsed time */
struct timeval tic, toc;

void argumentParser(int argc, char* argv[]) {
	if (argc != 5) {
		fprintf(stderr, "Error: invalid arguments\n");
		exit(EXIT_FAILURE);
	}
	
	inputfile = argv[1];
	outputfile = argv[2];
	numberThreads = atoi(argv[3]);
	syncStrategy = argv[4];
	
	if (numberThreads < 1) {
		fprintf(stderr, "Error: invalid number of threads\n");
		exit(EXIT_FAILURE);
	}

	else if (!strcmp(syncStrategy, "nosync") && numberThreads != 1) {
		fprintf(stderr, "Error: nosync only can be used with one thread\n");
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

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), file)) {
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
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
}

/* syncronization lock initializer */
void sync_locks_init() {
	pthread_mutex_init(&call_vector, NULL);
    
    if (!strcmp(syncStrategy, "mutex")) {
    	pthread_mutex_init(&mutex, NULL);
    }

    else if (!strcmp(syncStrategy, "rwlock")) {
    	pthread_rwlock_init(&rwlock, NULL);
    }
}

/* TecnicoFS content -> syncronization lock enabler */
void fs_lock(char token) {
    if (!strcmp(syncStrategy, "mutex")) {
        pthread_mutex_lock(&mutex);
    }

    else if (!strcmp(syncStrategy, "rwlock")) {
        if (token == 'd' || token == 'c') {
            pthread_rwlock_wrlock(&rwlock);
        }

        else if (token == 'l') {
            pthread_rwlock_rdlock(&rwlock);
        }
    }
}

/* TecnicoFS content -> syncronization lock disabler */
void fs_unlock() {
    if (!strcmp(syncStrategy, "mutex")) {
        pthread_mutex_unlock(&mutex);
    }

    else if (!strcmp(syncStrategy, "rwlock")) {
        pthread_rwlock_unlock(&rwlock);
    }
}

void applyCommands() {
   	while (numberCommands > 0) {
		/* lock acess to call vector */
    	pthread_mutex_lock(&call_vector);
		
		const char* command = removeCommand();

    	/* unlock acess to call vector */
		pthread_mutex_unlock(&call_vector);
		
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
                        fs_lock(token);
                        printf("Create file: %s\n", name);
                        create(name, T_FILE);
                        fs_unlock();
                        break;
                    case 'd':
                        fs_lock(token);
                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
                        fs_unlock();
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;

            case 'l': 
                fs_lock(token);
                searchResult = lookup(name);
                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);
                
                fs_unlock();
                break;

            case 'd':
                fs_lock(token);
                printf("Delete: %s\n", name);
                delete(name);
                fs_unlock();
                break;

            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* wrapper function, calling applyCommands */
void* fnThread() {
	applyCommands();
	return NULL;
}

/* process pool initializer and runner */
void processPool() {
	int i = 0;
	pthread_t tid[numberThreads];
   	 
    for (i = 0; i < numberThreads; i++) {
        if (pthread_create(&tid[i], NULL, fnThread, NULL) != 0) {
            fprintf(stderr, "Error: could not create threads\n");
            exit(EXIT_FAILURE);
        }
    }
	
	/* Get time after the initialization of the process pool */
	gettimeofday(&tic, NULL);

    for (i = 0; i < numberThreads; i++) {
		if (numberCommands < 0) {
			break;
		}
        pthread_join(tid[i], NULL);
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
  	
  	sync_locks_init();
	processPool();
	
	print_elapsed_time();

	/* print tree */
	FILE *output = openFile(outputfile, "w");
    print_tecnicofs_tree(output);
	fclose(output);

    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
