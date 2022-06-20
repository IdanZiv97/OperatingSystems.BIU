#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#define SHARED_FILE "to_srv.txt"
#define ERROR "ERROR_FROM_EX4\n"
#define MAX_OPERATOR_VALUE 4
#define MIN_OPERATOR_VALUE 1

void setUpSignalHandlers();
void receiveResult(int signum);
void errorHandler(int signum);
void timeoutHandler(int signum);
int accessSharedBuffer();
int getRandomNumber();


int main(int argc, char** argv) {
    //initialize the signal handlers
    signal(SIGALRM, timeoutHandler);
    signal(SIGUSR1, receiveResult);
    signal(SIGUSR2, errorHandler);
    // check if the number of args is valid
    if (5 != argc) {
        printf(ERROR);
        exit(-1);
    }
    int sharedFileFD = accessSharedBuffer();
    if (sharedFileFD == -1) {
        // failed to open shared file
        printf(ERROR);
        exit(-1);
    }
    // process request of client - here we validate the data
    int operator = atoi(argv[3]);
    if (operator > MAX_OPERATOR_VALUE || operator < MIN_OPERATOR_VALUE) {
        close(sharedFileFD);
        remove(SHARED_FILE);
        printf(ERROR);
        exit(-1);
    }
    int leftOperand = atoi(argv[2]);
    int rightOperand = atoi(argv[4]);
    //create the dock
    int myPID = (int) getpid();
    char requestDataBuffer[100];
    sprintf(requestDataBuffer, "%d\n%d\n%d\n%d\n", myPID, leftOperand, operator, rightOperand);
    write(sharedFileFD, requestDataBuffer, strlen(requestDataBuffer));
    //close the shared file
    close(sharedFileFD);
    //send signal to server that i want calculations - SIGUSR1
    int response = kill(atoi(argv[1]), SIGUSR2);
    // //set alarm to 30 sec
    alarm(30);
    //pause - not in a loop
    pause();

}

void setUpSignalHandlers() {
    signal(SIGALRM, timeoutHandler);
    signal(SIGUSR1, receiveResult);
    signal(SIGUSR2, errorHandler);
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
 * @brief In case of an error in the program this function will be invoked.
 * @param signum signal number invoking the function
 */
void errorHandler(int signum) {
    printf("ERROR_FROM_EX4\n");
    exit(-1);
}

void receiveResult(int signum) {
    alarm(0);
    char fileName[50] = "to_client_";
    char clientPID[20];
    sprintf(clientPID, "%d.txt", (int) getpid());
    strcat(fileName, clientPID);
    int clientFileFD = open(fileName, R_OK);
    //try to open the file
    if (-1 == clientFileFD) {
        printf(ERROR);
        exit(-1);
    }
    //try to read
    char buffer[100];
    if (-1 == read(clientFileFD, buffer, strlen(buffer))) {
        close(clientFileFD);
        remove(fileName);
    }
    printf("%s", buffer);
    //delete and release used resources
    if (-1 == close(clientFileFD) || -1 == remove(fileName)) {
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
int accessSharedBuffer() {
    int triesCounter = 0;
    int fd;
    while(1) {
        if (10 == triesCounter) {
            fd = -1;
            return fd;
        }
        fd = open(SHARED_FILE, O_WRONLY | O_CREAT | O_EXCL, 0777);
        if (fd < 0) {
            int timeToSleep = rand() % 6 + 1;
            sleep(timeToSleep);
        } else {
            break;
        }
    }
    return fd;
}



