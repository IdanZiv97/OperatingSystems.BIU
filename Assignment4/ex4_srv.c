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
#define ERROR "ERROR_FROM_EX4\n"
#define SHARED_FILE "to_srv.txt"
#define TIMEOUT_VALUE 60

typedef enum OPERATORS {
    ADD = 1,
    SUB = 2,
    MULT = 3,
    DIV = 4,
} OPERATORS;

void setUpSignalHandlers();
void handleRequest(int signum);
void timeoutHandler(int signum);
int calculate(int left, int op, int right);


int main(int argc, char** argv) {
    // set up signal handlers
    signal(SIGUSR2, handleRequest);
    signal(SIGALRM, timeoutHandler);
    // first delete to_srv if exists
    if (access(SHARED_FILE, F_OK) == 0) {
        if (remove(SHARED_FILE) == -1) {
            raise(SIGUSR2);
        }
    }
    // set the alarm
    while(1) {
        alarm(TIMEOUT_VALUE);
        pause();
    }
    return 0;
}

void handleRequest(int signum) {
    // cancel the curret alarm
    signal(SIGUSR2, handleRequest);
    alarm(0);
    //create child process
    int retVal = fork();
    if (retVal == -1) {
        printf(ERROR);
        exit(-1);
    } else if (retVal == 0) { // child
        FILE *toSrv = fopen(SHARED_FILE, "r");
        if (NULL == toSrv) {
            printf(ERROR);
            exit(-1);
        }
        // gather request data from shared file
        char buffer[20];
        int leftOpearnd, rightOperand , operator, clientPID;
        int iteration = 1;
        while(fgets(buffer, 20, toSrv) != NULL) {
            if (1 == iteration) {
                clientPID = atoi(buffer);
                iteration++;
            } else if (2 == iteration) {
                leftOpearnd = atoi(buffer);
                iteration++;
            } else if (3 == iteration) {
                operator = atoi(buffer);
                iteration++;
            } else { // 4 == iteration
                rightOperand = atoi(buffer);
                iteration++;
            }
        }
        // create client file
        char clientFilename[100] = "to_client_";
        char clientPIDstr[20];
        sprintf(clientPIDstr, "%d", clientPID);
        strcat(clientFilename, clientPIDstr);
        strcat(clientFilename, ".txt");
        int clientFD = open(clientFilename, O_CREAT | O_WRONLY, 0777);
        // perform calculation
        if (0 == rightOperand && DIV == operator) {
            write(clientFD, "CANNOT_DIVIDE_BY_ZERO\n", strlen("CANNOT_DIVIDE_BY_ZERO\n"));
        } else {
            int result = calculate(leftOpearnd, operator, rightOperand);
            char resultBuffer[20];
            sprintf(resultBuffer, "%d\n", result);
            write(clientFD, resultBuffer, strlen(resultBuffer));
        }
        //close the file
        close(clientFD);
        // notify the client
        kill(clientPID, SIGUSR1);
    } else { // father - reset the timer
        alarm(TIMEOUT_VALUE);
        return;
    }
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

int calculate(int left, int op, int right) {
    int result;
    switch (op) {
        case ADD:
            result = left + right;
            break;
        case SUB:
            result = left - right;
            break;
        case MULT:
            result = left * right;
            break;
        case DIV:
            result = left / right;
            break;
    }
    return result;
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
    printf("ERORR_FROM_EX4\n");
    exit(-1);
}

void setUpSignalHandlers() {
    signal(SIGUSR1, handleRequest);
    signal(SIGALRM, timeoutHandler);
}