// Idan Ziv 318175197
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
#define COMPARE "./comp.out"
#define ERROR(str) write(STDERR_FILENO, str, strlen(str));
#define ERR -1

void handle(char *mainDirectory, char *inputFile, char *outputFile);
int handleStudentFiles(char *studentDirPath, char *studentName, char *inputFilePath, char *outputFilePath, ssize_t resultsFD);
int isCFile(char fileName[]);
int compileCFile(char *filePath);
void executeCFile(char *inputPath);
void gradeStudent(char *studentName, char *grade, char *gradeDescription, ssize_t resultsFD);
int compareOutput(char *outputFilePath);

int main(int argc, char const *argv[])
{
    // get path to conifg file
    char pathToConigurationFiles[MAX_PATH] = {};
    strcpy(pathToConigurationFiles, argv[1]);
    // read the paths inside the configuration files
    ssize_t configurationFD = open(pathToConigurationFiles, O_RDONLY);
    if (configurationFD == ERR) {
        ERROR("Error in: open\n")
        close(configurationFD);
        exit(ERR);
    }
    //copy the data from the config file
    //read the whole loot and split with '\n'
    char buffer[MAX_LEN * 3] = {};
    if (read(configurationFD, &buffer, sizeof(buffer)) == ERR) {
        ERROR("Error in: open\n")
        close(configurationFD);
        exit(ERR);
    }
    char *mainDirectoryPath = strtok(buffer, "\n");
    //check for valid main dir
    if (access(mainDirectoryPath, F_OK) == ERR) {
        ERROR("Not a valid directory\n")
        close(configurationFD);
        exit(ERR);
    }
    char *inputFilePath = strtok(NULL, "\n");
    if (access(inputFilePath, F_OK) == ERR) {
        ERROR("Input file not exist\n")
        close(configurationFD);
        exit(ERR);
    }
    char *expectedOutputPath = strtok(NULL, "\n");
     if (access(expectedOutputPath, F_OK) == ERR) {
        ERROR("Output file not exist\n")
        close(configurationFD);
        exit(ERR);
    }
    // no further use for open files to we can close them
    if (close(configurationFD) == ERR) {
        ERROR("Error in: close\n")
        exit(ERR);
    }
    // main work is done here
    handle(mainDirectoryPath, inputFilePath, expectedOutputPath);
    // need to delete files
    return 0;
}

/**
 * @brief function that initiate the buisness logic
 * @param mainDirectory main directory path
 * @param inputFile input file
 * @param outputFile correct output path
 */
void handle(char *mainDirectory, char *inputFile, char *outputFile) {
    //create reference to main dir
    DIR* mainDirStream = opendir(mainDirectory);
    if (mainDirStream == NULL) {
        ERROR("Error in: opendir\n")
        exit(ERR);
    }
    //create results
    ssize_t resultsFD = open("results.csv", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    if (resultsFD == ERR) {
        ERROR("Error in: open\n")
        closedir(mainDirStream);
        exit(ERR);
    }
    //create erros
    ssize_t errorsFD = open("errors.txt", O_WRONLY | O_CREAT, 0777 | O_APPEND);
    if (errorsFD == ERR) {
        ERROR("Error in: open\n")
        closedir(mainDirStream);
        close(resultsFD);
        exit(ERR);
    }
    // write error to errors.txt
    if (dup2(errorsFD, STDERR_FILENO) == ERR) {
        ERROR("Error in: dup2\n")
        closedir(mainDirStream);
        close(resultsFD);
        close(errorsFD);
        exit(ERR);
    }
    if (close(errorsFD) == ERR) {
        ERROR("Error in: close\n");
        closedir(mainDirStream);
        close(resultsFD);
        exit(ERR);
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
            if (handleStudentFiles(currentSubDirPath, itemInFolder->d_name, inputFile, outputFile, resultsFD) != 1) {
                gradeStudent(itemInFolder->d_name, "0", "NO_C_FILE", resultsFD);
            }
        }
    }
    if (closedir(mainDirStream) == ERR) {
        ERROR("Error in: closedir\n");
        close(resultsFD);
        close(errorsFD);
        exit(ERR);
    }
    if (unlink("studentCompile.out") == ERR) {
        ERROR("Error in: unlink\n");
        close(resultsFD);
        close(errorsFD);
        exit(ERR);
    }
}

/**
 * @brief iterates a specific student's files. resposible of grading the students work
 * 
 * @param studentDirPath path to current student's directory
 * @param studentName
 * @param inputFilePath 
 * @param outputFilePath 
 * @param resultsFD student's grade will be written to this file
 * @return int 1 if the student had a C file, 0 if not
 */
int handleStudentFiles(char *studentDirPath, char *studentName, char *inputFilePath, char *outputFilePath, ssize_t resultsFD) {
    //try to open the student's folder
    DIR *currentDirStream = opendir(studentDirPath);
    if (currentDirStream == NULL) {
        ERROR("Error in: opendir\n");
        close(resultsFD);
        exit(ERR);
    }
    //iterate over the files and check for c file - one at most (hold a flag to check if the c file existed)
    int cFileExists = 0;
    struct dirent *itemInFolder;
    while ((itemInFolder = readdir(currentDirStream)) != NULL) {
        if (itemInFolder->d_type == DT_DIR) {
            continue;
        }
        if (itemInFolder->d_type == DT_REG) {
            if (isCFile(itemInFolder->d_name)) {
                //create path for file
                char cFilePath[MAX_PATH] = {};
                strcpy(cFilePath, studentDirPath);
                strcat(cFilePath, "/");
                strcat(cFilePath, itemInFolder->d_name);
                cFileExists = 1;
                int compilationResult = compileCFile(cFilePath);
                // compilation failed
                if (compilationResult == 0) {
                    gradeStudent(studentName, "10", "COMPILATION_ERROR", resultsFD);
                    continue;
                }
                // run c file and wait for result
                executeCFile(inputFilePath);
                int compareResult = compareOutput(outputFilePath);
                if (compareResult == 1) {
                    gradeStudent(studentName, "100", "EXCELLENT", resultsFD);
                    if (unlink("studentOutput.txt") == ERR) {
                        ERROR("Error in: unlink\n");
                        close(resultsFD);
                        exit(ERR);
                    }
                    continue;
                } else if (compareResult == 2) {
                    gradeStudent(studentName, "50", "WRONG", resultsFD);
                    if (unlink("studentOutput.txt") == ERR) {
                        ERROR("Error in: unlink\n");
                        close(resultsFD);
                        exit(ERR);
                    }
                    continue;
                } else if (compareResult == 3) {
                    gradeStudent(studentName, "75", "SIMILAR", resultsFD);
                    if (unlink("studentOutput.txt") == ERR) {
                        ERROR("Error in: unlink\n");
                        close(resultsFD);
                        exit(ERR);
                    }
                    continue;
                }
            }
        }
    }
    if (closedir(currentDirStream) == ERR) {
        ERROR("Error in: closedir\n");
        close(resultsFD);
        exit(ERR);
    }
    return cFileExists;
}


/**
 * @brief checks if a file is a C file
 * @param fileName
 * @return int return 1 if the the file is a C file, else it returns 0
 */
int isCFile(char fileName[]) {
    int len = strlen(fileName);
    if (fileName[len] == '\0' && fileName[len - 1] == 'c' && fileName[len - 2] == '.') {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief compiles a given C file
 * 
 * @param filePath path to the C file
 * @return int returns 1 if compilation is succesful, else it returns 0
 */
int compileCFile(char *filePath) {
    pid_t forkResult = fork();
    if (forkResult == ERR) {
        ERROR("Error in: fork\n");
        exit(ERR);
    } else if (forkResult == 0) { // child
        // create args for execvp
        char *args[MAX_LEN] = {
            "gcc",
            filePath,
            "-o",
            "studentCompile.out",
            NULL
        };
        if (execvp(args[0], args) == ERR) {
            ERROR("Error in: execvp in compile\n");
            exit(ERR);
        }
    } else { // father
        int status;
        if (waitpid(forkResult, &status, 0) == ERR) {
            ERROR("Error in: waitpid\n");
            exit(ERR);
        }
        // check for compilation error
        if (WEXITSTATUS(status) != 0) { //compilation failed
            return 0;
        } else { // file compiled successfully
            return 1;
        }
    }
}

/**
 * @brief executes a compiled C file
 * @param inputPath path to input file
 */
void executeCFile(char *inputPath) {
    pid_t forkResult = fork();
    if (forkResult == ERR) {
        ERROR("Error in: fork\n");
        exit(ERR);
    } else if (forkResult == 0) { // child
        //open input file
        ssize_t inputFileFD = open(inputPath, O_RDONLY);
        if (inputFileFD == ERR) {
            ERROR("Error in: open\n");
            exit(ERR);
        }
        // redirect input to stdin
        if (dup2(inputFileFD, STDIN_FILENO) == ERR) { //failed to redirect input
            ERROR("Error in: dup2\n");
            close(inputFileFD);
            exit(ERR);
        }
        //close input file
        if (close(inputFileFD) == ERR) {
            ERROR("Error in: close\n");
            exit(ERR);
        }
        //create output
        ssize_t outputFileFD = open("studentOutput.txt", O_WRONLY | O_CREAT, 0777);
        if (outputFileFD == ERR) {
            ERROR("Error in: open\n");
            exit(ERR);
        }
        //redirect ouput
        if (dup2(outputFileFD, STDOUT_FILENO) == ERR) {
            ERROR("Error in: dup2\n");
            close(outputFileFD);
            exit(ERR);
        }
        //close output file
        if (close(outputFileFD) == ERR) {
            ERROR("Error in: close\n");
        }
        //preprare args
        char *args[MAX_LEN] = {
            "./studentCompile.out",
            NULL
        };
        //execute
        if (execvp(args[0], args) == ERR) {
            ERROR("Error in: execvp\n");
            exit(ERR);
        }
    } else { //father
        int status;
        if (waitpid(forkResult, &status, 0) == ERR) {
            ERROR("Error in: waitpid\n");
            exit(ERR);
        }
    }
}

/**
 * @brief comparing the output of the student and the correct output file given in the config file
 * @param outputFilePath correct output path
 * @return int the comparison result of ./comp.out
 */
int compareOutput(char *outputFilePath) {
    pid_t forkResult = fork();
    if (forkResult == ERR) {
        ERROR("Error in: fork\n");
        exit(ERR);
    } else if (forkResult == 0) { // child
        //prepare args
        char *args[MAX_LEN] = {
            "./comp.out",
            outputFilePath,
            "studentOutput.txt",
            NULL
        };
        if (execvp(args[0], args) == ERR) {
            ERROR("Error in: execvp in compare\n");
            exit(ERR);
        }
    } else {
        int status;
        if (waitpid(forkResult, &status, 0) == ERR) {
            ERROR("Error in: waitpid\n");
            exit(ERR);
        }
        return WEXITSTATUS(status);
    }
}

/**
 * @brief Grades a student based on the result of ./comp.out
 * @param studentName 
 * @param grade 
 * @param gradeDescription 
 * @param resultsFD 
 */
void gradeStudent(char *studentName, char *grade, char *gradeDescription, ssize_t resultsFD) {
    //create a string buffer
    char str[MAX_LEN] = {};
    //create the format from the grade string from the given parameters
    strcpy(str, studentName);
    strcat(str, ",");
    strcat(str, grade);
    strcat(str, ",");
    strcat(str, gradeDescription);
    strcat(str, "\n");
    //write to results.csv
    if (write(resultsFD, str, strlen(str)) == ERR) {
        ERROR("Error in: write\n");
        close(resultsFD);
        exit(ERR);
    }
}