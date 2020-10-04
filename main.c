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

/* File opening with NULL checker */
FILE* openFile(char* name, char* mode) {
	FILE *fp = fopen(name, mode);
	
	/* Error check for inputfile */	
	if (fp == NULL) {	
		fprintf(stderr, "Error: could not open file\n");
    	exit(EXIT_FAILURE);
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

void processInput(char *name){
    char line[MAX_INPUT_SIZE];
    FILE *inputfile = openFile(name, "r");

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), inputfile)) {
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
	
	fclose(inputfile);
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
	
	/* checks if the number of arguments is correct */
	if (argc != 5) {
		fprintf(stderr, "Error: invalid arguments\n");
		exit(EXIT_FAILURE);
	}

    /* process input and print tree */
    processInput(argv[1]);
    
	/* A FAZER: TIMER */
	//clock_t start = clock();
	//clock_t finish = clock();
	//double elapsed = (double)(start - finish) / (double)(CLOCKS_PER_SEC);
	//printf("TecnicoFS completed in %.4f seconds.\n", elapsed);
	
	applyCommands();

	FILE *outputfile = openFile(argv[2], "w");
    print_tecnicofs_tree(outputfile);
	fclose(outputfile);

    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
