#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define ERROR "ERROR_FROM_EX4\n"
#define MAX_BUF 100
#define ALARM_VALUE 60
#define SHARED_FILE "to_srv.txt"
#define ADD 1
#define SUB 2
#define MULT 3
#define DIV 4

void handleRequest(int signum);
void execute();
void calculate(char* resultBuffer, int l, int o, int r);
void timeoutHandler(int signum);

int main() {
    signal(SIGUSR1, handleRequest);
    signal(SIGALRM, timeoutHandler);
    alarm(60);
    while(1) {
        pause();
    }
}

void handleRequest(int signum) {
    pid_t retVal = fork();
    if (0 == retVal) {
        execute();
        // alarm(0);
        exit(0);
    } else if (-1 == retVal) {
        printf("Server: fork faile\n");
        printf(ERROR);
        exit(-1);
    } else {
        alarm(60);
        signal(SIGUSR1, handleRequest);
    }
}

void execute() {
    FILE* sharedFile = fopen(SHARED_FILE, "r");
    if (NULL == sharedFile) {
        printf("Server: execute 1\n");
        printf(ERROR);
        exit(-1);
    }
    // read request data from file
    char clientPID[MAX_BUF];
    char leftOperandBuffer[MAX_BUF], operatorBuffer[MAX_BUF], rightOperandBuffer[MAX_BUF];
    int leftOperand, rightOperand, operator;
    pid_t clientPIDvalue;
    // read clientPID
    fgets(clientPID, MAX_BUF, sharedFile);
    //make sure the string ends with '\0'
    clientPID[strlen(clientPID) - 1] = '\0';
    clientPIDvalue = atoi(clientPID);
    //read left operand
    fgets(leftOperandBuffer, MAX_BUF, sharedFile);
    leftOperandBuffer[strlen(leftOperandBuffer) - 1] = '\0';
    leftOperand = atoi(leftOperandBuffer);
    // read operator
    fgets(operatorBuffer, MAX_BUF, sharedFile);
    operatorBuffer[strlen(operatorBuffer) - 1] = '\0';
    operator = atoi(operatorBuffer);
    // read rightopeator
    fgets(rightOperandBuffer, MAX_BUF, sharedFile);
    rightOperandBuffer[strlen(rightOperandBuffer) - 1] = '\0';
    rightOperand = atoi(rightOperandBuffer);
    // release and remove shared file - no need for it
    if (0 != fclose(sharedFile) || -1 == remove(SHARED_FILE)) {
        printf("Server: execute 2\n");
        printf(ERROR);
        exit(-1);
    }
    char result[MAX_BUF];
    calculate(result, leftOperand, operator, rightOperand);
    // create file name
    char clientFilename[MAX_BUF];
    sprintf(clientFilename, "to_client_%s.txt", clientPID);
    FILE* clientFile = fopen(clientFilename, "w+");
    if (NULL == clientFile) {
        printf("Server: execute 3\n");
        printf(ERROR);
        exit(-1);
    }
    fwrite(result, strlen(result), 1, clientFile);
    if (0 != fclose(clientFile)) {
        printf("Server: execute 4\n");
        printf(ERROR);
        exit(-1);
    }
    // kill(clientPIDvalue ,SIGUSR1);
}

void calculate(char* resultBuffer, int l, int o, int r) {
    if (0 == r && DIV == o) {
        strcpy(resultBuffer, "CANNOT_DIVIDE_BY_ZERO");
        return;
    }
    int result;
    switch (o) {
    case ADD:
        result = l + r;
        break;
    case SUB:
        result = l - r;
        break;
    case MULT:
        result = l * r;
        break;
    case DIV:
        result = l / r;
        break;
    }
    sprintf(resultBuffer, "%d", result);
}

void timeoutHandler(int signum) {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    int status;
    while(wait(&status) != -1){}
    exit(0);
}