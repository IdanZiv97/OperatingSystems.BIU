#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "stdlib.h"
#include "fcntl.h"
#include "unistd.h"
#include "time.h"
#include "string.h"

#define ERROR "ERROR_FROM_EX4\n"
#define SHARED_FILE "to_srv.txt"
#define MAX_BUF 100
#define ALARM_VALUE 60

void handleResponse(int signum);
void timeoutHandler(int signum);

int main(int argc, char** argv) {
    if (argc != 5) {
        printf(ERROR);
        exit(-1);
    }
    signal(SIGUSR1, handleResponse);
    signal(SIGALRM, timeoutHandler);
    FILE* sharedFile;
    int i;
    int tries = 0;
    for (i = 0; i < 10; i++) {
        sharedFile = fopen(SHARED_FILE, "w+");
        if (NULL == sharedFile) {
            sleep(rand() % 6 + 1);
            tries++;
        } else {
            break;
        }
    }
    if (10 == tries) {
        printf("Client: main1\n");
        printf(ERROR);
        exit(-1);
    }
    // write data
    char myPIDstr[MAX_BUF];
    sprintf(myPIDstr, "%u", getpid());
    fwrite(myPIDstr, strlen(myPIDstr), 1, sharedFile);
    fwrite("\n", strlen("\n"), 1, sharedFile);
    // write down the calculation parameters
    for (i = 2; i < 5; i++) {
        fwrite(argv[i], strlen(argv[i]), 1, sharedFile);
        fwrite("\n", strlen("\n"), 1, sharedFile);
    }
    if (0 != fclose(sharedFile)) {
        printf("Client: main2\n");
        printf(ERROR);
        exit(-1);
    }
    alarm(ALARM_VALUE);
    pid_t serverPID = atoi(argv[1]);
    kill(serverPID, SIGUSR1);
    pause();
    return 0;
}

void handleResponse(int signum) {
    alarm(0);
    char myFilename[100];
    sprintf(myFilename, "to_client_%u.txt", getpid());
    FILE* myFile = fopen(myFilename, "r");
    if (NULL == myFile) {
        printf("Client: handle1\n");
        printf(ERROR);
        exit(-1);
    }
    char buffer[MAX_BUF];
    fgets(buffer,MAX_BUF, myFile);
    printf("%s\n", buffer);
    if (0 != fclose(myFile) || -1 == remove(myFilename)) {
        printf("Client: handle2\n");
        printf(ERROR);
        exit(-1);
    }
}

void timeoutHandler(int signum) {
    printf("Client closed because no response was recived from the server for 30 seconds\n");
    exit(-1);
}