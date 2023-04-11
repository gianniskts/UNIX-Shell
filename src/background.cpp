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

bool checkBackground(char** tokens) {
    // check for background process
    bool background = false;

    int i = 0;
    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "&") == 0) {
            background = true;
            tokens[i] = NULL;
            break;
        }
        i++;
    }

    // if background process, fork and execute
    if (background) {
        // fork and execute

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    } else if (pid == 0) {
        // child process
        execvp(tokens[0], tokens);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // parent process
        printf("Background process started with PID %d\n", pid);
    }
    }

    return background;
}