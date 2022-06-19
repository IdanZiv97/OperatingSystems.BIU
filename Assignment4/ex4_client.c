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
#include <linux/random.h>

typedef enum bool {
    false=0,
    true=1
}bool;

#define SHARED_FILE "to_srv"
#define MAX_OPERATOR_VALUE 4
#define MIN_OPERATOR_VALUE 1
#define STRING_BUF_SIZE 100

void setUpSignalHandlers();
void receiveResult(int signum);
void errorHandler(int signum);
bool processRequest(int* requestParameters, char** rawData, int sizeOfRawData);
void writeToServerFile(int* params, int sizeOfParams, int pid, int fd);
void timeoutHandler(int signum);


int main(int argc, char** argv) {
    //initialize the signal handlers
    setUpSignalHandlers();
    //check if the number of args is valid
        // else - raise error
    if (!5 == argc) {
        raise(SIGUSR2);
    }
    //process request of client - here we validate the data
    int params[4];
    char** clientData = argv + 1;
    if (processRequest(params, clientData, 4) == false) {
        raise(SIGUSR2);
    }
    //write the data to the shared buffer
    int sharedBufferFD;
    accessSharedBuffer(&sharedBufferFD);
    char** requestParams = argv + 2;
    writeToServerFile(params, 3, getpid(), sharedBufferFD);
    if (sharedBufferFD == -1) {
        // failed to open shared file
        raise(SIGUSR2);
    }
    //send signal to server that i want calculations - SIGUSR1
    kill(params[0], SIGUSR1);
    //set alarm to 30 sec
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
    if (-1 == read(clientFileFD, buffer, STRING_BUF_SIZE) {
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
 * @brief Here we will process the data of the client's request and put it inside the parameters belonging to the server
 * The function also validates.
 * @param requestParameters an array to hold the parameters we need to write to the shared file
 * @param rawData the arguments that the client code received
 * @param sizeOfRawData 
 * @return true in case the data is valid
 * @return false in case the data is invalid
 */
bool processRequest(int* requestParameters, char** rawData, int sizeOfRawData) {
    long int convert;
    bool isValid = true;
    for (int index = 0; index < sizeOfRawData; index++) {
        convert = strtol(rawData[index], NULL, 0);
        // making sure the conversion was successful
        if (EINVAL == errno) {
            isValid = false;
        }
        // check that we have an int - since the params are integers
        if (INT_MAX <= convert || INT_MIN >= convert) {
            isValid = false;
        }
        // no need to convert back to int - since we are in range of int so no data loss occurs
        requestParameters[index] = convert;
    }
    return isValid;
}

/**
 * @brief this function tries to gain access to the shared file
 * Because of the possibility that another client is trying to access the shared file of the server
 * we want to wait for a random period of time between tries.
 * We also want to limit our tries.
 * @param fd 
 */
void accessSharedBuffer(int* fd) {
    int triesCounter = 0;
    int randomNumbers[10];
    int result = syscall(SYS_getrandom, randomNumbers, sizeof(int) * 10, 0);
    if (-1 == result) {
        raise(SIGUSR2);
    }
    while(true) {
        if (10 == triesCounter) {
            raise(SIGUSR2);
        }
        int retVal = open(SHARED_FILE, O_WRONLY | O_CREAT | O_EXCL, 0777);
        if (retVal < 0) {
            int timeToSleep = (randomNumbers[triesCounter++] % 6) + 1;
            sleep(timeToSleep);
        } else {
            break;
        }
    }
    *fd = retVal;
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
    for (int index = 0; i < sizeOfParams; index++) {
        strcat(dataString, " ");
        strcat(dataString, params[index]);
    }
    if (write (fd, dataString, dataString, STRING_BUF_SIZE) == -1) {
        close(fd);
        raise(SIGUSR2);
    }
    close(fd);
}

void 
