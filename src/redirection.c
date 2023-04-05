#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/redirection.h"

bool append = false;

void checkRedirection(char* args[], bool* in_redirect, bool* out_redirect, char** in_file, char** out_file) {
    
    append = false;
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], "<") == 0) {
            *in_redirect = true;
            *in_file = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            *out_redirect = true;
            *out_file = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], ">>") == 0) {
            *out_redirect = true;
            *out_file = args[i + 1];
            args[i] = NULL;
            append = true;
        }

        i++;
        
    }
}

void handleRedirection(bool in_redirect, bool out_redirect, char* in_file, char* out_file) {
    if (in_redirect) {
        int fd_in = open(in_file, O_RDONLY);
        if (fd_in == -1) {
            perror("In redirect open");
            exit(1);
        }
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            perror("In redirect dup2");
            exit(1);
        }
        close(fd_in);
    }
    if (out_redirect) {
        // if ">", replace file, if ">>", append to file
        int flags = O_WRONLY | O_CREAT;
        if (append) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        int fd_out = open(out_file, flags, 0644);
        if (fd_out == -1) {
            perror("Out redirect open");
            exit(1);
        }
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("Out redirect dup2");
            exit(1);
        }
        close(fd_out);
    }
}