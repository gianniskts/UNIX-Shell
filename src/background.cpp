#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/background.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command

bool checkBackground(char** tokens, pid_t* pid) {
    // check for background process
    bool background = false; // flag to indicate if the process is a background process

    int i = 0;
    while (tokens[i] != NULL) { // iterate through the tokens to find the background symbol
        if (strcmp(tokens[i], "&") == 0) {
            background = true;
            tokens[i] = NULL;
            break;
        }
        i++;
    }

    // if background process, fork and execute
    if (background) {
        *pid = fork();
        if (*pid < 0) {
            perror("fork failed");
        } else if (*pid == 0) {
            // child process
            execvp(tokens[0], tokens);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else {
            // parent process
            printf("Background process started with PID %d\n", *pid);
            // doesnt wait for child process to finish because it runs on background
        }
    }

    return background;
}

void checkFinishedBackground(pid_t bg_pid, bool background) {
    while (background) {
        int status;
        pid_t result = waitpid(bg_pid, &status, WNOHANG);
        if (result == bg_pid) {
            // background process has finished
            printf("Background process with PID %d has finished\n", bg_pid);
            break;
        } else if (result == -1) {
            // error
            perror("waitpid failed");
            break;
        }
        // background process is still running
        sleep(1); // wait for a second before checking again
    }
}