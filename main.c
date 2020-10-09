#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // CONFIRMAR SE PODEMOS USAR ISTO
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* Filenames for the inputfile and outputfile */
char* outputfile = NULL;
char* inputfile = NULL;

/* Syncronization strategy */
char* syncStrategy = NULL;

/* Parser for the arguments */
void argumentParser(int argc, char* argv[]) {
	/* checks if the number of arguments is correct */
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
	
	/* Error check for inputfile */	
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
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(FILE *file){
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

void applyCommands(){
    while (numberCommands > 0){
        const char* command = removeCommand();
        if (command == NULL){
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

int main(int argc, char* argv[]) {
    /* init filesystem */
    init_fs();
	
	/* parsing arguments */
	argumentParser(argc, argv);

    /* process input */
    FILE* input = openFile(inputfile, "r");
	processInput(input);
	fclose(input);
    
	/* A FAZER: TIMER */
	//clock_t start = clock();
	//clock_t finish = clock();
	//double elapsed = (double)(start - finish) / (double)(CLOCKS_PER_SEC);
	//printf("TecnicoFS completed in %.4f seconds.\n", elapsed);
	
	applyCommands();

	/* print tree */
	FILE *output = openFile(outputfile, "w");
    print_tecnicofs_tree(output);
	fclose(output);

    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
