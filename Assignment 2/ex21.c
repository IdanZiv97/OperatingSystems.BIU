#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define IDENTICAL 1
#define SIMILAR 3
#define DIFFERENT 2
#define PATH_MAX 1024
    // intilise
        // pointer to flag called identical
    //loop - start
        // while chars equal continue read
        // if not equal
            // 1. read until not white space - for both spaces
            // 2. to lower case

    // go throught reminder - if different return different
    // if identical - return identical
    // else similiar
/**
 * This function reads the file to the next non space char or EOF 
 **/
void skipWhiteSpaces(int fileFD, char* buffer, int* fileReadIndicator);
/**
 * Handles the reminder
 * In case the reminder has something other than whitespaces it will check for the same char
 * If the char is different it will return DIFFERENT, else return SIMILAR
 **/
int handleReminder(int file1FD, char* buffer1, int* file1ReadIndicator, int file2FD, char* buffer2, int* file2ReadIndicator);

void closeFiles(int file1FD, int file2FD);


int main(int argc, char const *argv[])
{
    if (argc != 2) {
        perror("Not enough arguments\n");
        exit(0);
    }
    char file1Path[PATH_MAX] = {};
    strcpy(file1Path, argv[0]);
    char file2Path[PATH_MAX] = {};
    strcpy(file2Path, argv[1]);
    int file1FD = open(file1Path, O_RDONLY);
    if (file1FD == -1) {
        perror("Failed to open file 1\n");
    }
    int file2FD = open(file2Path, O_RDONLY);
    if (file2FD == -1) {
        perror("Failed to open file 1\n");
    }
    // buffer for read input
    char buffer1 = 0;
    char buffer2 = 0;
    int isIdentical = 1;
    int file1ReadIndicator = read(file1FD, &buffer1, 1);
    if (file1ReadIndicator == -1) {
        perror("Failed to read from file 1\n");
        closeFiles(file1FD, file2FD);
        exit(0);
    }
    int file2ReadIndicator = read(file2FD, &buffer2, 1);
    if (file2ReadIndicator == -1) {
        perror("Failed to read from file 2\n");
        closeFiles(file1FD, file2FD);
        exit(0);
    }
    while (file1ReadIndicator > 0 && file2ReadIndicator > 0) {
        while (buffer1 == buffer2) {
            file1ReadIndicator = read(file1FD, &buffer1, 1);
            if (file1ReadIndicator == -1) {
                perror("Failed to read from file 1\n");
                closeFiles(file1FD, file2FD);
                exit(0);
            }
            file1ReadIndicator = read(file1FD, &buffer1, 1);
            if (file1ReadIndicator == -1) {
                perror("Failed to read from file 1\n");
                closeFiles(file1FD, file2FD);
                exit(0);
            }
        }
        if (isIdentical) {isIdentical = 0;}
        skipWhiteSpaces(file1FD, &buffer1, &file1ReadIndicator);
        skipWhiteSpaces(file2FD, &buffer2, &file2ReadIndicator);
        if (tolower((int) buffer1) != tolower((int) buffer2)) {
            return DIFFERENT;
        }
        file1ReadIndicator = read(file1FD, &buffer1, 1);
        if (file1ReadIndicator == -1) {
            perror("Failed to read from file 1\n");
            closeFiles(file1FD, file2FD);
            exit(0);
        }
        file1ReadIndicator = read(file1FD, &buffer1, 1);
        if (file1ReadIndicator == -1) {
            perror("Failed to read from file 1\n");
            closeFiles(file1FD, file2FD);
            exit(0);
        }
    }
    // handleReminder
    if (file1ReadIndicator > 0) {
        isIdentical = 0;
        skipWhiteSpaces(file1FD, &buffer1, &file1ReadIndicator);
        if (file1ReadIndicator != 0) {
            return DIFFERENT;
        }
    }
    if (file2ReadIndicator > 0) {
        isIdentical = 0;
        skipWhiteSpaces(file2FD, &buffer2, &file2ReadIndicator);
        if (file2ReadIndicator != 0) {
            return DIFFERENT;
        }
    }

    if (isIdentical) { 
        return IDENTICAL;
    } else {
        return SIMILAR;
    }
    return 0;
}
