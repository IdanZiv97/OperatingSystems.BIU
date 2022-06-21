#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>

#define SHARED_FILE "to_srv.txt"
#define ERROR "ERROR_FROM_EX4\n"
#define BUF_MAX 100
#define ALARM_VALUE 30

void setUpSignalHandlers();
void receiveResult(int signum);
void errorHandler(int signum);
void timeoutHandler(int signum);
FILE* accessSharedBuffer();
int getRandomNumber();
void writeRequestData(FILE* f, char** params);


int main(int argc, char** argv) {
    //initialize the signal handlers
    setUpSignalHandlers();
    // check if the number of args is valid
    if (5 != argc) {
        printf(ERROR);
        exit(-1);
    }
    FILE* sharedFile = accessSharedBuffer();
    if (NULL == sharedFile) {
        // failed to open shared file
        printf(ERROR);
        exit(-1);
    }
    writeRequestData(sharedFile, argv);

    pid_t serverPID = atoi(argv[1]);
    kill(serverPID, SIGUSR1);
    alarm(ALARM_VALUE);
    pause();
    return 0;
}

/**
 * assigning user defined handler for signals
 */
void setUpSignalHandlers() {
    signal(SIGALRM, timeoutHandler);
    signal(SIGUSR1, receiveResult);
}

/**
 * @brief The client code is waiting for 30 seconds to a response from the server
 * In case the server response has not been recieved in that time span the client code
 * will exit with error and print a proper message
 * @param signum signal number invoking the function
 */
void timeoutHandler(int signum) {
    printf("Client closed because no response was received from the server in 30 seconds");
    exit(-1);
}

/**
 * @brief the client processes the arguments passed into concreted data to the server
 * @param f shared file
 * @param params arguments passed from the user
 */
void writeRequestData(FILE* f, char** params) {
    char myPID[BUF_MAX];
    sprintf(myPID, "%u", getpid());
    fwrite(myPID, strlen(myPID), 1, f);
    fwrite("\n", strlen("\n"), 1, f);
    // write calculation params
    int i;
    for (i = 2; i < 5; i++) {
        fwrite(params[i], strlen(params[i]), 1, f);
        fwrite("\n", strlen("\n"), 1, f);
    }
    // close the file
    if (0 != fclose(f)) {
        printf(ERROR);
        exit(-1);
    }
}


/**
 * @brief handles response from the server
 * @param signum 
 */
void receiveResult(int signum) {
    alarm(0);
    char fileName[BUF_MAX];
    sprintf(fileName, "to_client_%u.txt", getpid());
    FILE* myFile = fopen(fileName, "r");
    //try to open the file
    if (NULL == myFile) {
        printf(ERROR);
        exit(-1);
    }
    //try to read
    char result[BUF_MAX];
    fgets(result, BUF_MAX, myFile);
    printf("%s\n", result);
    //delete and release used resources
    if (0 != fclose(myFile) || -1 == remove(fileName)) {
        printf(ERROR);
        exit(-1);
    }
}

/**
 * @brief this function tries to gain access to the shared file
 * Because of the possibility that another client is trying to access the shared file of the server
 * we want to wait for a random period of time between tries.
 * We also want to limit our tries.
 * @return if failed to access file than fd is -1
 */
FILE* accessSharedBuffer() {
    int triesCounter = 0;
    FILE* file;
    while(1) {
        if (10 == triesCounter) {
            file = NULL;
            return file;
        }
        file = fopen(SHARED_FILE, "w");
        if (NULL == file) {
            int timeToSleep = rand() % 6 + 1;
            sleep(timeToSleep);
        } else {
            break;
        }
    }
    return file;
}



