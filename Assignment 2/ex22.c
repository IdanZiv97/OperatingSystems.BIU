#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>


// Constatnts
#define MAX_LEN 151 // 150 + 1 for the new line char
#define MAX_PATH 150
#define COMPARE "./compt.out"

//function to compile C file - returns error or not
handle(char *mainDirectory, char *inputFile, char *outputFile);
//function to handle a student
handleStudentFiles(char *studentDirPath, char *studentName, char *inputFilePath, char *outputFilePath, ssize_t resultsFD, ssize_t errorsFD);
int isCFile(char fileName[]);
int compileCFile(char *filePath ,ssize_t errorsFD);
int executeCFile(char *inputPath, ssize_t errorsFD);
void gradeStudent(char *studentName, char *grade, char *gradeDescription, ssize_t resultsFD);

int main(int argc, char const *argv[])
{
    // get path to conifg file
    char pathToConigurationFiles[MAX_PATH] = {};
    strcpy(pathToConigurationFiles, argv[1]);
    // read the paths inside the configuration files
    ssize_t configurationFD = open(pathToConigurationFiles, O_RDONLY);
    if (configurationFD == -1) {
        write(STDERR_FILENO, "Error in: open\n", strlen("Error in: open\n"));
        close(configurationFD);
        exit(-1);
    }
    //copy the data from the config file
    //read the whole loot and split with '\n'
    char buffer[MAX_LEN * 3] = {};
    if (read(configurationFD, &buffer, sizeof(buffer)) == -1) {
        write(2, "Error in: read\n", strlen("Error in: read\n"));
        close(configurationFD);
        exit(-1);
    }
    char *mainDirectoryPath = strtok(buffer, "\n");
    //check for valid main dir
    if (access(mainDirectoryPath, F_OK) == -1) {
        write(2, "Not a valid directory\n", strlen("Not a valid directory\n"));
        close(configurationFD);
        exit(-1);
    }
    char *inputFilePath = strtok(NULL, "\n");
    if (access(inputFilePath, F_OK) == -1) {
        write(2, "Input file not exist\n", strlen("Input file not exist\n"));
        close(configurationFD);
        exit(-1);
    }
    char *expectedOutputPath = strtok(NULL, "\n");
     if (access(expectedOutputPath, F_OK) == -1) {
        write(2, "Output file not exist\n", strlen("Output file not exist\n"));
        close(configurationFD);
        exit(-1);
    }
    // no further use for open files to we can close them
    close(configurationFD);
    // main work is done here
    handle(mainDirectoryPath, inputFilePath, expectedOutputPath);
    return 0;
}

handle(char *mainDirectory, char *inputFile, char *outputFile) {
    //create reference to main dir
    DIR* mainDirStream = opendir(mainDirectory);
    if (mainDirStream == NULL) {
        write(2, "Error in: opendir\n", strlen("Error in: opendir\n"));
        exit(-1);
    }
    //create results
    ssize_t resultsFD = open("results.csv", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    if (resultsFD == -1) {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        closedir(mainDirStream);
        exit(-1);
    }
    //create erros
    ssize_t errorsFD = open("errors.txt", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    if (errorsFD == -1) {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        closedir(mainDirStream);
        close(resultsFD);
    }
    // fetch all the items in the directory
    struct dirent *itemInFolder;
    while ((itemInFolder = readdir(mainDirStream)) != NULL) {
        // if it is a regular file continue
        if (itemInFolder->d_type == DT_REG) {
            continue;
        }
        // if it is "." or ".." skip
        if (strcmp(itemInFolder->d_name, ".") == 0 || strcmp(itemInFolder->d_name, "..") == 0) {
            continue;
        }
        // the file is a directory - create path to it
        if (itemInFolder->d_type == DT_DIR) {
            char currentSubDirPath[MAX_PATH] = {};
            strcpy(currentSubDirPath, mainDirectory);
            strcat(currentSubDirPath, "/");
            strcat(currentSubDirPath, itemInFolder->d_name);
            handleStudentFiles(currentSubDirPath, itemInFolder->d_name, inputFile, outputFile, resultsFD, errorsFD);       
        }
    }
}

handleStudentFiles(char *studentDirPath, char *studentName, char *inputFilePath, char *outputFilePath, ssize_t resultsFD, ssize_t errorsFD) {
    //try to open the student's folder
    DIR *currentDirStream = opendir(studentDirPath);
    if (currentDirStream == NULL) {
        write(2, "Error in: opendir\n", strlen("Error in: opendir\n"));
        close(resultsFD);
        close(errorsFD);
        exit(-1);
    }
    //iterate over the files and check for c file - one at most (hold a flag to check if the c file existed)
    int cFileExists = 0;
    struct dirent* itemInFolder;
    while ((itemInFolder = readdir(currentDirStream) != NULL) && !cFileExists) {
        if (itemInFolder->d_type == DT_DIR) {
            continue;
        }
        if(strcmp(itemInFolder->d_name, ".") || )
        if (itemInFolder->d_type == DT_REG) {
            if (isCFile(itemInFolder->d_name)) {
                //create path for file
                char cFilePath[MAX_PATH] = {};
                strcpy(cFilePath, studentDirPath);
                strcat(cFilePath, "/");
                strcat(cFilePath, itemInFolder->d_name);
                cFileExists = 1;
                int compilationResult = compileCFile(cFilePath, errorsFD);
                // compilation failed
                if (compilationResult == 0) {
                    gradeStudent(studentName, "20", "COMPILATION_ERROR", resultsFD);
                    continue;
                }
                // run c file and wait for result
                int execiteResult = executeCFile()
            }
        }
    }
    //compile c file
    //check for result in compilation
    // if failed grade and leave
    // if not exectue and compare
    // grade based on ./comp.out result
}


/**
 * @brief function to check if a file is c file.
 * each c file ends with ".c\0" so lets check if the string ".cֿֿ\0" is a substring of the file name
 * @param fileName 
 * @return int 1 if true 0 if false
 */
int isCFile(char fileName[]) {
    char *tmp = strstr(fileName, ".c\0");
    if (tmp == NULL) {
        return 0;
    } else {
        return 0;
    }
}

/**
 * @brief function to compile c files
 * 
 * @param filePath 
 * @param errorsFD 
 * @return int 
 */
int compileCFile(char *filePath ,ssize_t errorsFD) {
    pid_t forkResult = fork();
    if (forkResult == -1) {
        write(2, "Error in: fork\n", strlen("Error in: fork\n"));
        close(errorsFD);
    } else if (forkResult == 0) { // child
        // create args for execvp
        char *args[] = {
            "gcc",
            filePath,
            "-o",
            "student.out",
            NULL
        };
        if (execvp(args[0], args) == -1) {
            write(2, "Error in: execvp\n", strlen("Error in: execvp\n"));
            close(errorsFD);
            exit(-1);
        }
    } else { // father
        int status;
        if (waitpid(forkResult, &status, 0) == -1) {
            write(2, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            close(errorsFD);
            exit(-1);
        }
        // check for compilation error
        if (WIFEXITED(status) == 0) { //compilation failed
            // write error to errors.txt
            if (dup2(errorsFD, 2) == -1) {
                write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
                close(errorsFD);
                exit(-1);
            }
            return 0;
        } else { // file compiled successfully
            return 1;
        }
    }
}

int executeCFile(char *inputPath, ssize_t errorsFD) {
    int forkResult = fork();
    if (forkResult == -1) {
        write(2, "Error in: fork\n", strlen("Error in: fork\n"));
        close(errorsFD);
        exit(-1);
    } else if (forkResult == 0) { // child
        //open input file
        ssize_t inputFileFD = open(inputPath, O_RDONLY);
        if (inputFileFD == -1) {
            write(2, "Error in: open\n", strlen("Error in: open\n"));
            close(errorsFD);
            exit(-1);
        }
        // redirect input to stdin
        if (dup2(inputFileFD, 0) == -1) { //failed to redirect input
            write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
            close(errorsFD);
            close(inputFileFD);
            exit(-1);
        }
        if (close(inputFileFD) == -1) {
            write(2, "Error in: close\n", strlen("Error in: close\n"));
            close(errorsFD);
            exit(-1);
        }
        //create output

    }   
}

/**
 * @brief writes the grade to results.csv
 * 
 * @param studentName 
 * @param grade 
 * @param gradeDescription 
 * @param resultsFD 
 */
void gradeStudent(char *studentName, char *grade, char *gradeDescription, ssize_t resultsFD) {
    //create a string buffer
    char *str;
    //create the format from the grade string from the given parameters
    strcpy(str, studentName);
    strcat(str, ",");
    strcat(str, grade);
    strcat(str, ",");
    strcat(str, gradeDescription);
    //write to results.csv
    write(resultsFD, str, strlen(str));
}