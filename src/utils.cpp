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

bool checkSemicolon(char* users_command) {
    int len = strlen(users_command);
    for (int i = 0; i < len; i++) {
        if (users_command[i] == ';') {
            return true;
        }
    }
    return false;
}

bool handleMultipleCommands(char* users_command) {
    users_command[strlen(users_command) - 1] = '\0';

    // Split input by semicolons
    char* command = strtok(users_command, ";");
    while (command != NULL) {
        // Split command by spaces to extract command name and arguments
        char* args[MAX_ARGS];
        char* arg = strtok(command, " ");
        int i = 0;
        while (arg != NULL) {
            args[i++] = arg;
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;  // Set last argument to NULL
        
        // Fork a new process
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(args[0], args);
            perror("execvp");  // Should not reach this point if execvp succeeds
            exit(1);
        } else if (pid > 0) {
            // Parent process
            int status;
            waitpid(pid, &status, 0);  // Wait for child process to complete
        } else {
            // Error
            perror("fork");
            exit(1);
            return false;
        }
        
        command = strtok(NULL, ";");  // Get next command
    }

    return true;
}

