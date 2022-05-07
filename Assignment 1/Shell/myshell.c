#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
// Global Constants
#define MAX_LENGTH 100
#define MAX_CMD 100
// Global Variables
typedef struct Command {
    pid_t commandPID;
    char commandString[MAX_LENGTH];
}Command;

Command commandHistory[100];
int numOfCommands = 0;


// function to print the prompt and handle user input
void shellPrompt(char inputBuffer[MAX_LENGTH]);
// function to add path to the env variable path
void addToPATH(const char* path);
//function to handle cd
void cd(char path[MAX_LENGTH]);
//function to print history
void history();


int main(int argc, char const *argv[]) {
    // handle adding the paths passed as arguments to PATH
    printf("num of args: %d\n", argc);
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            addToPATH(argv[i]);
        }
    }
    // declare and initialize buffer for user input
    char userInput[MAX_LENGTH] = {};
    shellPrompt(userInput);
    while (strcmp(userInput, "exit") != 0) {
        // support the behavior of a shell when no input entered or a space
        if (strcmp(userInput, "") == 0 || isspace(userInput)) {
            continue;
        }
        //handle user input - break to arguements
        char* inputCopy[MAX_LENGTH] = {};
        // the copy will allow us to save the original input for history
        strcpy(inputCopy, userInput);
        char *tokens[MAX_LENGTH] = {};
        int numOfTokens = 0;
        char *token;
        token = strtok(inputCopy, " ");
        while (token) {
            tokens[numOfTokens] = token;
            token = strtok(NULL, " ");
            numOfTokens++;
        }
        // now we have the input
        //first check with the buitin - history and cd
        //history
        if (strcmp(tokens[0], "history") == 0) {
            history();
            shellPrompt(userInput);
            continue;
        }
        //cd
        if (strcmp(tokens[0], "cd") == 0) {
            // add the command to the history
            commandHistory[numOfCommands].commandPID = getpid();
            strcpy(commandHistory[numOfCommands].commandString, userInput);
            numOfCommands++;
            cd(tokens[1]);
            shellPrompt(userInput);
            continue;
        }
        // a non build in command was entered
        // fork a new process
        pid_t forkResult = fork();
        // check for succuesful fork
        if (forkResult == -1) {
            perror("fork failed");
        } else if (forkResult == 0) { // execute child - run execv
            if (execvp(tokens[0], tokens) == -1) {
                perror("execvp failed");
            }
        } else { // execute parent
            int childStatus;
            if (wait(&childStatus) == -1) {
                perror("wait failed");
            } else {
                if (WIFEXITED(childStatus)) {
                    commandHistory[numOfCommands].commandPID = forkResult;
                    strcpy(commandHistory[numOfCommands].commandString, userInput);
                    numOfCommands++;
                }
            }
        }
        shellPrompt(userInput);
    }
    
}

void shellPrompt(char inputBuffer[MAX_LENGTH]) {
    printf("$ ");
    fflush(stdout);
    fgets(inputBuffer, MAX_LENGTH, stdin);
    // get rid of newline character from fgets
    char *newLineCharacter = strchr(inputBuffer, '\n');
    if (newLineCharacter != NULL) {
        *newLineCharacter = 0;
    }
}

void addToPATH(const char* path) {
    //get the current PATH
    char *currentPATH = getenv("PATH");
    if (currentPATH == NULL) {
        printf("currentPath is NULL\n");
        return;
    }
    //concat using strcat
    strcat(currentPATH, ":");
    strcat(currentPATH, path);
    //set the new path
    int setenvStatus = setenv("PATH", currentPATH, 1);
    if (setenvStatus == -1) {
        perror("setenv failed");
    }
}

/**
 * This function handles the cd command.
 * The command is built in so there is no need for it to run on a different process.
 * @path the path desired
 * 
 **/
void cd(char path[MAX_LENGTH]) {
    if (chdir(path) == -1) {
        perror("chdir failed");
    }
}

/**
 * This function prints all the past commands
 **/
void history() {
    //first add this current history
    commandHistory[numOfCommands].commandPID = getpid();
    strcpy(commandHistory[numOfCommands].commandString, "history");
    numOfCommands++;
    for (int index = 0; index < numOfCommands; index++) {
        pid_t currentPID = commandHistory[index].commandPID;
        char *currentString = commandHistory[index].commandString;
        printf("%ld %s\n", currentPID, currentString);
    }
 }

