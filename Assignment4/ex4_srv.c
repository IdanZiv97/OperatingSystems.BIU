#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>

#define MAX_BUF 100
#define ERROR "ERROR_FROM_EX4\n"
#define SHARED_FILE "to_srv.txt"
#define ALARM_VALUE 60
#define ADD 1
#define SUB 2
#define MULT 3
#define DIV 4

void setUpSignalHandlers();
void handleRequest(int signum);
void timeoutHandler(int signum);
void calculate(char* res, int left, int op, int right);
void execute();
void readRequestData(FILE* f, int* params, char* clientPID);


int main(int argc, char** argv) {
    // set up signal handlers
    signal(SIGUSR1, handleRequest);
    signal(SIGALRM, timeoutHandler);
    // first delete to_srv if exists
    if (0 == access(SHARED_FILE, F_OK)) {
        if (-1 == remove(SHARED_FILE)) {
            printf(ERROR);
        }
    }
    // set the alarm
    alarm(ALARM_VALUE);
    while(1) {
        pause();
    }
    return 0;
}

void execute() {
    FILE* sharedFile = fopen(SHARED_FILE, "r");
    if (NULL == sharedFile) {
        printf(ERROR);
        exit(-1);
    }
    int params[4];
    char clientPID[MAX_BUF];
    readRequestData(sharedFile, params, clientPID);

    char result[MAX_BUF];
    calculate(result, params[1], params[2], params[3]);

    char clientFilename[MAX_BUF];
    sprintf(clientFilename, "to_client_%s.txt", clientPID);
    FILE* clientFile = fopen(clientFilename, "w");
    if (NULL == clientFile) {
        printf(ERROR);
        exit(-1);
    }
    fwrite(result, strlen(result), 1, clientFile);
    if (0 != fclose(clientFile)) {
        printf(ERROR);
        exit(-1);
    }
    pid_t clientPIDvalue = params[0];
    kill(clientPIDvalue, SIGUSR1);
}

void readRequestData(FILE* f, int* params, char* clientPID) {
    char buffer[100];
    // read client PID
    fgets(clientPID, MAX_BUF, f);
    clientPID[strlen(clientPID) - 1] = '\0';
    params[0] = atoi(clientPID);
    // read left operand
    fgets(buffer, MAX_BUF, f);
    buffer[strlen(buffer) - 1] = '\0';
    params[1] = atoi(buffer);
    //read operator
    fgets(buffer, MAX_BUF, f);
    buffer[strlen(buffer) - 1] = '\0';
    params[2] = atoi(buffer);
    // read right operand
    fgets(buffer, MAX_BUF, f);
    buffer[strlen(buffer) - 1] = '\0';
    params[3] = atoi(buffer);
    if (0 != fclose(f) || -1 == remove(SHARED_FILE)) {
        printf(ERROR);
        exit(-1);
    }
}

void handleRequest(int signum) {
    // cancel the curret alarm
    signal(SIGUSR1, handleRequest);
    alarm(0);
    //create child process
    int retVal = fork();
    if (retVal == -1) {
        printf(ERROR);
        exit(-1);
    } else if (retVal == 0) { // child
        execute();
        exit(0);
    } else { // father - reset the timer
        alarm(ALARM_VALUE);
        signal(SIGUSR1, handleRequest);
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

void calculate(char* res, int left, int op, int right) {
    if (0 == right && DIV == op) {
        sprintf(res, "%s", "CANNOT_DIVIDE_BY_ZERO");
        return;
    }
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
    sprintf(res, "%d", result);
}




/**
 * @brief 
 * 
 * @param signum 
 */
void timeoutHandler(int signum) {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    int status;
    while(wait(&status) != -1){}
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