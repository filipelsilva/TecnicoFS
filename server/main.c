#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
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

/* Socket parameters */
char* serverName;
int sockfd;

/* Syncronization lock */
pthread_mutex_t mutex;

/* Conditional syncronization */
pthread_cond_t other_cond, print_cond;
int is_printing = 0;

/* Initializes locks */
void sync_locks_init() {
    if (pthread_mutex_init(&mutex, NULL)) {
        fprintf(stderr, "Error: could not initialize mutex: mutex\n");
    }

    if (pthread_cond_init(&other_cond, NULL)) {
        fprintf(stderr, "Error: could not initialize condition: other_cond\n");
    }

    if (pthread_cond_init(&print_cond, NULL)) {
        fprintf(stderr, "Error: could not initialize condition: print_cond\n");
    }
}

/* Destroys locks */
void sync_locks_destroy() {
    if (pthread_cond_destroy(&print_cond)) {
        fprintf(stderr, "Error: could not destroy mutex: print_cond\n");
    }

    if (pthread_cond_destroy(&other_cond)) {
        fprintf(stderr, "Error: could not destroy condition: other_cond\n");
    }

    if (pthread_mutex_destroy(&mutex)) {
        fprintf(stderr, "Error: could not destroy mutex: mutex\n");
    }
}

/* Call vector -> syncronization lock enabler */
void mutex_lock() {
    if (pthread_mutex_lock(&mutex)) {
        fprintf(stderr, "Error: could not lock mutex: mutex\n");
    }
}


/* Call vector -> syncronization lock disabler */
void mutex_unlock() {
    if (pthread_mutex_unlock(&mutex)) {
        fprintf(stderr, "Error: could not unlock mutex: mutex\n");
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

/* Broadcasts (to all threads) the conditional wait to move on */
void broadcast(pthread_cond_t* cond) {
    if (pthread_cond_broadcast(cond)) {
        fprintf(stderr, "Error: could not broadcast the condition variable\n");
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

int fsMount() {
    struct sockaddr_un server_addr;
    socklen_t addrlen;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "server: can't open socket\n");
        exit(EXIT_FAILURE);
    }

    unlink(serverName);
    addrlen = setSockAddrUn(serverName, &server_addr);

    if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
        fprintf(stderr, "server: bind error\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int fsUnmount() {
    if (close(sockfd) < 0){
        fprintf(stderr, "client: close error \n");
        exit(EXIT_FAILURE);
    }

    if (unlink(serverName) < 0){
        fprintf(stderr, "client: unlink error \n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int applyCommands(const char* command) {
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
                    return create(name, T_FILE);
                case 'd':
                    return create(name, T_DIRECTORY);
                default:
                    fprintf(stderr, "Error: invalid node type\n");
                    exit(EXIT_FAILURE);
            }

        case 'l':
            searchResult = lookup(name);
            if (searchResult >= 0) {
                printf("Search: %s found\n", name);
                return searchResult;
            }
            else {
                printf("Search: %s not found\n", name);
                return searchResult;
            }

        case 'd':
            return delete(name);

        case 'm':
            return move(name, type);

        default: { /* error */
                     fprintf(stderr, "Error: command to apply\n");
                     exit(EXIT_FAILURE);
                 }
    }

    exit(EXIT_FAILURE);
}

int applyOther(const char* command) {
    mutex_lock();
    int answer;

    while (is_printing) {
        cond_wait(&other_cond, &mutex);
    }

    answer = applyCommands(command);

    signal(&print_cond);
    mutex_unlock();
    return answer;
}

int applyPrint(const char* command) {
    mutex_lock();

    while (!is_printing) {
        cond_wait(&print_cond, &mutex);
    }

    char token;
    char filename[MAX_INPUT_SIZE];
    sscanf(command, "%c %s", &token, filename);
    int answer;

    answer = print(filename);

    is_printing --;

    signal(&other_cond);
    mutex_unlock();

    return answer;
}


void *receiveCommands() {
    struct sockaddr_un client_addr;
    socklen_t addrlen;
    char in_buffer[INDIM];
    int c;
    int answer;

    addrlen = sizeof(struct sockaddr_un);

    while(1) {
        c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
                (struct sockaddr *)&client_addr, &addrlen);

        if (c <= 0) continue;

        in_buffer[c]='\0';

        if (in_buffer[0] == 'p') {
            mutex_lock();
            is_printing++;
            mutex_unlock();
            printf("%s\n", in_buffer);
            answer = applyPrint(in_buffer);
        }

        else {
            printf("%s\n", in_buffer);
            answer = applyOther(in_buffer);
        }

        if (sendto(sockfd, &answer, sizeof(int), 0, (struct sockaddr *) &client_addr, addrlen) < 0) {
            fprintf(stderr,"client: sendto error\n");
            exit(EXIT_FAILURE);
        }

    }
    return NULL;
}

/* process pool initializer and runner */
void processPool() {
    int i;
    pthread_t tid[numberThreads];

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

}

int main(int argc, char* argv[]) {
    /* init filesystem */
    init_fs();

    argumentParser(argc, argv);

    /* Create server socket*/
    fsMount();

    sync_locks_init();

    processPool();

    sync_locks_destroy();

    fsUnmount();

    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
