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
void skipWhiteSpaces(int fileFD, char* buffer);

void closeFiles(int file1FD, int file2FD);


int main(int argc, char const *argv[])
{
    if (argc != 3) {
        perror("Not enough arguments\n");
        exit(0);
    }
    char file1Path[PATH_MAX] = {};
    strcpy(file1Path, argv[1]);
    char file2Path[PATH_MAX] = {};
    strcpy(file2Path, argv[2]);
    ssize_t file1FD = open(file1Path, O_RDONLY);
    if (file1FD == -1) {
        perror("Failed to open file 1\n");
    }
    ssize_t file2FD = open(file2Path, O_RDONLY);
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
        if (buffer1 != buffer2) {
            isIdentical = 0;
            skipWhiteSpaces(file1FD, &buffer1);
            skipWhiteSpaces(file2FD, &buffer2);
            buffer1 = (unsigned char) tolower(buffer1);
            buffer2 = (unsigned char) tolower(buffer2);
            if (buffer1 != buffer2) {
                closeFiles(file1FD, file2FD);
                return DIFFERENT;
            }
        }
        file1ReadIndicator = read(file1FD, &buffer1, 1);
        if (file1ReadIndicator == -1) {
            perror("failed to read from file\n");
            closeFiles(file1FD, file2FD);
        }
        file2ReadIndicator = read(file2FD, &buffer2, 1);
        if (file2ReadIndicator == -1) {
            perror("failed to read from file\n");
            closeFiles(file1FD, file2FD);
        }
    }
    // handleReminder - at this point the files are either similar or different. depends if the reminders conatain
    // a non-whitespace character
    if (file1ReadIndicator > 0) {
        isIdentical = 0;
        skipWhiteSpaces(file1FD, &buffer1);
        if (!isspace(buffer1)) {
            return DIFFERENT;
        }
    }
    if (file2ReadIndicator > 0) {
        isIdentical = 0;
        skipWhiteSpaces(file2FD, &buffer2);
        if (!isspace(buffer2)) {
            return DIFFERENT;
        }
    }

    if (isIdentical) { 
        closeFiles(file1FD, file2FD);
        return IDENTICAL;
    } else {
        closeFiles(file1FD, file2FD);
        return SIMILAR;
    }
}


void skipWhiteSpaces(int fileFD, char* buffer) {
    ssize_t readResult;
    while (isspace(*buffer)) {
        readResult = read(fileFD, buffer, 1);
        // handle case where we reached EOF
        if (readResult == 0) {
            break;
        }
    }
}

void closeFiles(int file1FD, int file2FD) {
    if (close(file1FD) == -1) {
        perror("failed closing files");
        exit(0);
    }
    if (close(file2FD) == -1) {
        perror("failed closing files");
        exit(0);
    }
}