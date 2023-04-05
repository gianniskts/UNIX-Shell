#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/pipe.h"

void checkPipe(char* args[], bool* pipe_present, int* pipe_index) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            *pipe_present = true;
            *pipe_index = i;
            break;
        }
    }
}

void handlePipe(char** args, int pipe_index) {
    int pipefd[2];
    pid_t pid;

    // create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    // fork a child process to execute the left-hand side of the pipe
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // close the write end of the pipe
        close(pipefd[0]);
        // redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        // execute the left-hand side of the pipe
        args[pipe_index] = NULL;
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(1);
        }
    }

    // fork another child process to execute the right-hand side of the pipe
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // close the read end of the pipe
        close(pipefd[1]);
        // redirect stdin to the read end of the pipe
        dup2(pipefd[0], STDIN_FILENO);
        // execute the right-hand side of the pipe
        args += pipe_index + 1;
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(1);
        }
    }

    // close the pipe file descriptors in the parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // wait for both child processes to finish executing
    wait(NULL);
    wait(NULL);

}