#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>

#define NUM_OF_ARGS 4
#define STRING_BUF_SIZE 100
#define SHARED_FILE "to_srv"
#define TIMEOUT_VALUE 60
typedef enum bool {
    false=0,
    true=1
} bool;

void setUpSignalHandlers();
void handleRequest(int signum);
void errorHandler(int signum, siginfo_t *info, void *ptr);
void timeoutHandler(int signum);
void performRequest();
bool calculate(int leftOperand, int rightOperand, int operator, int* product);

int main(int argc, char** argv) {
    // set up signal handlers
    // first delete to_srv if exists
    if (access(SHARED_FILE, F_OK) == 0) {
        if (remove(SHARED_FILE) == -1) {
            raise(SIGUSR2);
        }
    }
    // set the alarm
    alarm(TIMEOUT_VALUE);
    while(true) {
        pause();
    }
    return 0;
}

void handleRequest(int signum) {
    // cancel the curret alarm
    alarm(0);
    //create child process
    int retVal = fork();
    if (retVal == -1) {
        raise(SIGUSR2);
    } else if (retVal == 0) { //child
        performRequest();
    } else { // father - reset the timer
        alarm(TIMEOUT_VALUE);
    }
}

void performRequest() {
    int requestParams[4] = {};
    int result;
    int clientFilename[STRING_BUF_SIZE] = "to_client_";
    // try and open the shared file
    int sharedFileFD = open(SHARED_FILE, R_OK, 0777);
    if (-1 == sharedFileFD) { // child process failed, invoke error handler at the father processor
        kill(getppid() ,SIGUSR2);
        exit(-1);
    }
    //read the raw request data
    char buffer[STRING_BUF_SIZE] = {};
    if (read(sharedFileFD, buffer, STRING_BUF_SIZE) == -1) {
        kill(getppid(), SIGUSR2);
        exit(-1);
    }
    // try to process the request that the client wrote to the shared file
    if (processRequest(requestParams, buffer, 4) == false) {
        close(sharedFileFD);
        remove(SHARED_FILE);
        kill(getppid(), SIGUSR2);
        exit(-1);
    }
    // close un used resources
    if (close(sharedFileFD) == -1 || remove(SHARED_FILE) == -1) {
        kill(getppid(), SIGUSR2);
        exit(-1);
    }
    //create client file
    char clientPID[STRING_BUF_SIZE] = {};
    sprintf(clientPID, "%d", requestParams[0]);
    strcat(clientFilename, clientPID);
    int clientFileFD = open(clientFilename, O_CREAT | O_WRONLY, 0777);
    if (clientFileFD == -1) {
        kill(getppid(), SIGUSR2);
        exit(-1);
    }
    // calculate
    int result;
    char clientResult[STRING_BUF_SIZE] = {};
    if (calculate(requestParams[1], requestParams[2], requestParams[3], &result) == true) {
        sprintf(clientResult, "%d", result);
        write(clientFileFD, clientResult, strlen(clientResult));
    } else { // need to write en error
        write(clientFileFD, "ERROR: Can't divide by 0\n", strlen("ERROR: Can't divide by 0\n"));
    }
    close(clientFileFD);
    kill(requestParams[0], SIGUSR1);
    kill(getppid(), SIGUSR2);
    exit(0);
}

bool processRequest(int *params, char* rawData, int sizeOfParams) {
    char *token = strtok(rawData, " ");
    long int convert = strtol(token, NULL, 0);
    bool isSuccesful = true;
    // making sure the conversion was successful
    if (EINVAL == errno) {
        isSuccesful = false;
    }
    // check that we have an int - since the params are integers
    if (INT_MAX <= convert || INT_MIN >= convert) {
        isSuccesful = false;
    }
    int index = 0;
    params[index++] = convert;
    for (; index < sizeOfParams; index++) {
        token = strtok(NULL, " ");
        convert = strtol(token, NULL, 0);
        // making sure the conversion was successful
        if (EINVAL == errno) {
            isSuccesful = false;
            break;
        }
        // check that we have an int - since the params are integers
        if (INT_MAX <= convert || INT_MIN >= convert) {
            isSuccesful = false;
            break;
        }
        // no need to cast to int - since we are in range of int so no data loss occurs
        params[index] = convert;
    }
    return isSuccesful;
}

/**
 * @brief performs the desired arithmetic operation from the client
 * @param leftOperand 
 * @param rightOperand
 * @param operator
 * @param calculation 
 * @return true
 * @return false 
 */

bool calculate(int leftOperand, int rightOperand, int operator, int* calculation) {
    bool isSucsessful = true;
    switch (operator) {
    case 1:
        *calculation = leftOperand + rightOperand; 
        break;
    case 2:
        *calculation = leftOperand - rightOperand;
    case 3:
        *calculation = leftOperand * rightOperand;
        break;
    case 4:
        if (rightOperand == 0) {
            isSucsessful = false;
        } else {
            *calculation = leftOperand / rightOperand;
        }
        break;
    }
    return isSucsessful;
}




/**
 * @brief 
 * 
 * @param signum 
 */
void timeoutHandler(int signum) {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    exit(0);
}


/**
 * @brief The errors can only occur in the 
 * 
 * @param signum 
 * @param info 
 * @param ptr 
 */
void errorHandler(int signum, siginfo_t *info, void *ptr) {
    int retStatus;
    if (info->si_pid != getpid()) {
        waitpid(info->si_pid, &retStatus, 0);
        if (WIFEXITED(retStatus)) {
            if (WEXITSTATUS(retStatus) == -1) {
                printf("ERORR_FROM_EX4");
                exit(-1);
            }
        }
    }
}

void setUpSignalHandlers() {
    struct sigaction usrAction;
    sigset_t blockMask;
    sigfillset(&blockMask);
    usrAction.sa_sigaction = errorHandler;
    usrAction.sa_flags = blockMask;
    usrAction.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &usrAction, NULL);
    signal(SIGUSR1, handleRequest);
    signal(SIGALRM, timeoutHandler);
}