//looked for help for basic shell:
    // https://www.geeksforgeeks.org/making-linux-shell-c/
    // https://blog.ehoneahobed.com/building-a-simple-shell-in-c-part-1
    // https://brennan.io/2015/01/16/write-a-shell-in-c/
// Token: https://www.tutorialspoint.com/string-tokenisation-function-in-c
// chdir: https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
// fork: https://www.geeksforgeeks.org/fork-system-call/
// access: https://linux.die.net/man/2/access
// execv: https://linuxhint.com/c-execve-function-usage/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//define global variables
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 64

char* arguments[MAX_ARGUMENTS]; //pointer for arguments
int numArguments = 0; //initializing number of arguments

int background = 0; //initializing background execution number

//function to check if the given line is background function or not
int backgroundExecution(const char* token) {
    return (token != NULL && strlen(token) == 1 && token[0] == '!');
}

//tokenize input
void parseInput(char* line) {
    numArguments = 0;
    background = 0;

    char* token = strtok(line, " \t\n");

    while (token != NULL) {
        // check if supposed to run on background
        if (backgroundExecution(token)) {
            background = 1;
        } else {
            // if not a nackground token set new argument and increase number of arguments
            arguments[numArguments++] = token;
        }
        token = strtok(NULL, " \t\n");
    }

    arguments[numArguments] = NULL;

    // Handle special case for "cd" command
    if (numArguments > 0 && strcmp(arguments[0], "cd") == 0) {
        // Add an extra argument to hold the command itself
        for (int i = numArguments; i > 0; --i) {
            arguments[i] = arguments[i - 1];
        }

        arguments[0] = "cd";
        numArguments++;
    }
}

//execute build in commands within the shell
void builtInCommand() {
    // check if command is exit and exit shell
    if (strcmp(arguments[0], "exit") == 0) {
        exit(0);
    }
    // check if command is cd
    else if (strcmp(arguments[0], "cd") == 0) {
        // check that cd has valid number of arguments
        if (numArguments == 1) {
            printf("cd: missing argument\n");
        }
        else if (numArguments > 3) {
            printf("cd: too many arguments\n");
        }
        else {
            // change the directory using chdir()
            if (chdir(arguments[1]) != 0) {
                printf("cd: failed to change directory\n");
            }
        }
    }
}

void externalCommand() {
    // creaste new process
    pid_t pid = fork();

    // check if new(=child) process was succesfully created
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    // if was continue with this
    } else if (pid == 0) {
        // check if comman is accessable with acces function
        if (access(arguments[0], X_OK) == 0) {
            // if it is executes it with execv
            execv(arguments[0], arguments);
        } else {
            // else error occurs
            fprintf(stderr, "Command not found: %s\n", arguments[0]);
            exit(1);
        }
    } else {
        // Parent process check wether the process is supposed to run in back or not
        if (!background) {
            wait(NULL);
        }
    }
}


int main(int argc, char *argv[]) {

    // check amount of arguments given
    if (argc == 2) {
        // tries open file
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Failed to open file: %s\n", argv[1]);
            exit(1);
        }

        char line[MAX_COMMAND_LENGTH];
        // if file opening ok then read file line by line and parse it to token
        while (fgets(line, sizeof(line), file)) {
         parseInput(line);

            if (numArguments > 0) {
                // break while loop if first argument given is exit
                if (strcmp(arguments[0], "exit") == 0) {
                    break;
                }
                // check that first argument given is not empty string
                if (arguments[0][0] != '\0') {
                    //
                    if (strcmp(arguments[0], "cd") == 0) {
                        builtInCommand();
                    } else {
                        externalCommand();
                    }
                }
            }
        }

        fclose(file);
    // if no commanline arguments read commands from user
    } else {
        while (1) {
            printf("<wish> ");
            // ensure that promt is displayed
            fflush(stdout);

            char line[MAX_COMMAND_LENGTH];
            if (fgets(line, sizeof(line), stdin) == NULL) {
                exit(0);
            }
        //tokenizing
         parseInput(line);

            if (numArguments > 0) {
                if (strcmp(arguments[0], "exit") == 0) {
                    break;
                }

                if (arguments[0][0] != '\0') {
                    // if argument is not empty check if its path if it is use built in command
                    if (strcmp(arguments[0], "cd") == 0) {
                        builtInCommand();
                    } else {
                        externalCommand();
                    }
                }
            }
        }
    }

    return 0;
}