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
        char* tokens[MAX_ARGS];
        char* token = strtok(command, " ");
        int i = 0;
        while (token != NULL) {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }
        tokens[i] = NULL;  // Set last argument to NULL

        // Check for background process
        pid_t bg_pid; // pid of the background process
        bool background = false;
        background = checkBackground(tokens, &bg_pid); // check if the command is a background process
if (background) {
    // Fork a new process for the background command
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(tokens[0], tokens);
        perror("execvp"); // Should not reach this point if execvp succeeds
        exit(1);
    } else {
        // Parent process
        // printf("[%d] %d\n", ++num_jobs, pid); // print the job number and PID of the background process
        // bg_jobs[num_jobs].pid = pid; // store the PID of the background process
        // bg_jobs[num_jobs].status = RUNNING; // set the status of the background process to running
        
        bg_pid = pid; // assign the pid of the background process to bg_pid
    }
} else {
    // Not a background process, fork and execute
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // if (redirect_input || redirect_output) { // if there is input/output redirection
        //     handleRedirection(redirect_input, redirect_output, input_file, output_file, append_output);
        // }
        execvp(tokens[0], tokens); // execute the command
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for child process to complete
    } else {
        // Error
        perror("fork");
        exit(1);
        return false;
    }
    
    bg_pid = 0; // reset bg_pid if it was set previously
}
        // Check for finished background processes
        checkFinishedBackground(bg_pid, background);

        command = strtok(NULL, ";");  // Get next command
    }

    return true;
}

