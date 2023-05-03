#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "../include/utils.h"
#include "../include/redirection.h"
#include "../include/pipe.h"
#include "../include/background.h"
#include "../include/alias.h"
#include "../include/signals.h"
#include "../include/history.h"


#define MAX_ARGS 10

void parseCommand(char** tokens, char* command) {
    char* token = strtok(command, " \n"); // tokenize the command string

    int i = 0;
    while (token != NULL) { // loop through each token in the command
        glob_t paths; // a glob_t structure to store the matched paths
        int flags = 0;

        if (strchr(token, '*') != NULL || strchr(token, '?') != NULL) { // check if the token contains wildcards
            flags |= GLOB_TILDE; // set the GLOB_TILDE flag to expand tilde characters
            glob(token, flags, NULL, &paths); // expand the wildcard pattern using glob

            // loop through the matching paths and add them to the tokens array
            for (int j = 0; j < paths.gl_pathc; j++) {
                tokens[i++] = strdup(paths.gl_pathv[j]);
            }

            globfree(&paths); // free the memory allocated by glob
        } else {
            tokens[i++] = strdup(token); // add the token to the tokens array as it is
        }

        token = strtok(NULL, " \n"); // move to the next token
    }

    tokens[i] = NULL; // add a NULL terminator to the end of the tokens array
}

// check if there is a semicolon in the command
bool checkSemicolon(char* users_command) {
    int len = strlen(users_command);
    for (int i = 0; i < len; i++) {
        if (users_command[i] == ';') {
            return true;
        }
    }
    return false;
}

// handle multiple commands separated by semicolons
bool handleMultipleCommands(char* users_command) {
    users_command[strlen(users_command) - 1] = '\0'; // remove the newline character from the end of the command

    // Split input by semicolons
    char* command = strtok(users_command, ";"); // get the first command with delimiter ';'
    while (command != NULL) { // loop through each command
        // Split command by spaces to extract command name and arguments
        char* tokens[MAX_ARGS];
        char* token = strtok(command, " ");
        int i = 0;
        while (token != NULL) {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }
        tokens[i] = NULL;  // Set last argument to NULL

        // check for background process
            pid_t bg_pid; // pid of the background process
            bool background = checkBackground(tokens, &bg_pid); // check if the command is a background process
            checkFinishedBackground(bg_pid, background); // check if the background process has finished

            
            if (!background) { 
                // execute the command with input/output redirection
                pid_t pid = fork(); 
                running_pid = pid; // store the pid of the running process

                if (pid == 0) { // child process

                        execvp(tokens[0], tokens); // execute the command
                        perror("execvp");
                        exit(1);
                } else {
                    // parent process
                    int status;
                    waitpid(pid, &status, 0); // Wait for child process to complete

                }
            } else {
                // continue with the next command
                command = strtok(NULL, ";");
                continue;
                    
            }
        command = strtok(NULL, ";");  // Get next command
    }

    return true;
}

