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
// #include <linux/random.h>


typedef enum bool {
    false=0,
    true=1
}bool;

#define SHARED_FILE "to_srv.txt"
#define ERROR "ERROR_FROM_EX4\n"
#define MAX_OPERATOR_VALUE 4
#define MIN_OPERATOR_VALUE 1

#define STRING_BUF_SIZE 100
#define TIMEOUT_VALUE 30

void setUpSignalHandlers();
void receiveResult(int signum);
void errorHandler(int signum);

void timeoutHandler(int signum);
int accessSharedBuffer();
int getRandomNumber();


int main(int argc, char** argv) {
    //initialize the signal handlers
    setUpSignalHandlers();
    // check if the number of args is valid
    if (5 != argc) {
        printf(ERROR);
        exit(-1);
    }
    int sharedFileFD = accessSharedBuffer();
    if (sharedFileFD == -1) {
        // failed to open shared file
        raise(SIGUSR2);
    }
    // process request of client - here we validate the data
    //send signal to server that i want calculations - SIGUSR1
    // kill(atoi(argv[1]), SIGUSR1);
    // //set alarm to 30 sec
    // alarm(30);
    // //pause - not in a loop
    // pause();
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
    //cancle current alarm request
    alarm(0);
    char fileName[STRING_BUF_SIZE] = "to_client_";
    char clientPID[STRING_BUF_SIZE] = {};
    sprintf(clientPID, "%d", getpid());
    strcat(fileName, clientPID);
    int clientFileFD = open(fileName, R_OK);
    //try to open the file
    if (-1 == clientFileFD) {
        raise(SIGUSR2);
    }
    //try to read
    char buffer[STRING_BUF_SIZE] = {};
    if (-1 == read(clientFileFD, buffer, STRING_BUF_SIZE)) {
        close(clientFileFD);
        remove(fileName);
        raise(SIGUSR2);
    }
    //print the result
    printf("%s\n", buffer);
    //delete and release used resources
    if (-1 == close(clientFileFD) || -1 == remove(fileName)) {
        raise(SIGUSR2);
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
    while(true) {
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


/**
 * @brief This function print the calculation request data to the shared file
 * 
 * @param params the request data
 * @param sizeOfParams
 * @param pid the pid of the owner of the request
 * @param fd the fd of the shared buffer file
 */

void writeToServerFile(char** params, int sizeOfParams, int pid, int fd) {
    char dataString[STRING_BUF_SIZE] = {};
    char pidBuffer[STRING_BUF_SIZE] = {};
    sprintf(pidBuffer, "%d", pid);
    for (int index = 0; index < sizeOfParams; index++) {
        strcat(dataString, " ");
        strcat(dataString, params[index]);
    }
    if (write(fd, dataString, strlen(dataString)) == -1) {
        close(fd);
        raise(SIGUSR2);
    }
    close(fd);
}
