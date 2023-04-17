#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/redirection.h"

using namespace std;

void checkRedirection(char** tokens, bool* redirect_input, bool* redirect_output, char** input_file, char** output_file, int* append_output) {
    int j = 0;
    while (tokens[j] != NULL) {
        if (strcmp(tokens[j], "<") == 0) {
            *redirect_input = true;
            *input_file = tokens[j + 1];
            tokens[j] = NULL;
        } else if (strcmp(tokens[j], ">") == 0) {
            *redirect_output = true;
            *output_file = tokens[j + 1];
            tokens[j] = NULL;
        } else if (strcmp(tokens[j], ">>") == 0) {
            *redirect_output = true;
            *output_file = tokens[j + 1];
            *append_output = true;
            tokens[j] = NULL;
        }
        j++;
    }
}

void handleRedirection(bool redirect_input, bool redirect_output, char* input_file, char* output_file, bool append_output) {
    if (redirect_input) {
        // redirect input from a file
        int fd = open(input_file, O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (redirect_output) {
        // redirect output to a file
        int fd;
        if (append_output) {
            fd = open(output_file, O_CREAT | O_WRONLY | O_APPEND, 0666);
        } else {
            fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        }
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
} 