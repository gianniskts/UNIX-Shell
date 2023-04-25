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

void checkRedirection(char** tokens, bool* redirect_input, bool* redirect_output, char** input_file, char** output_file, bool* append_output) {
    int j = 0;
    while (tokens[j] != NULL) {
        if (strcmp(tokens[j], "<") == 0) { 
            *redirect_input = true; // needs to redirect input
            *input_file = tokens[j + 1]; // the file to redirect input from
            tokens[j] = NULL; // set the token to NULL so that execvp() doesn't try to execute it
        } else if (strcmp(tokens[j], ">") == 0) {
            *redirect_output = true; // needs to redirect output
            *output_file = tokens[j + 1]; // the file to redirect output to
            tokens[j] = NULL; // set the token to NULL so that execvp() doesn't try to execute it
        } else if (strcmp(tokens[j], ">>") == 0) {
            *redirect_output = true; // needs to redirect output
            *output_file = tokens[j + 1]; // the file to redirect output to
            *append_output = true; // needs to append output to the file
            tokens[j] = NULL; // set the token to NULL so that execvp() doesn't try to execute it
        }
        j++;
    }
}

void handleRedirection(bool redirect_input, bool redirect_output, char* input_file, char* output_file, bool append_output) {
    if (redirect_input) {
        // redirect input from a file
        int fd = open(input_file, O_RDONLY); // uses this flag to open the file for reading only
        if (fd == -1) { 
            perror("open");
            exit(1);
        }
        dup2(fd, STDIN_FILENO); // duplicate the file descriptor fd to the standard input
        close(fd); 
    }
    if (redirect_output) {
        // redirect output to a file
        int fd;
        if (append_output) { // if >> is used, append to the file
            fd = open(output_file, O_CREAT | O_WRONLY | O_APPEND, 0666); // append to the file
        } else {
            fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666); // truncate the file
        }
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
} 