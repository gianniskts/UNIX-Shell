#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "include/redirection.h"
#include "include/pipe.h"

#define MAX_LINE 80 // The maximum length command 

int main(void) {

    char *args[MAX_LINE / 2 + 1]; // command line arguments
    int should_run = 1;           // flag to determine when to exit program

    while (should_run) {

        printf("in-mysh-now:>");
        fflush(stdout);

        // read input from the user 
        char command[MAX_LINE];
        fgets(command, MAX_LINE, stdin);

        // parse the command into tokens 
        char* token = strtok(command, " \n");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " \n");
        }
        args[i] = NULL;

        // check for redirection 
        bool in_redirect = false; 
        bool out_redirect = false;
        char* in_file = NULL;
        char* out_file = NULL;

        bool pipe_present = false;
        int pipe_index = -1;

        checkPipe(args, &pipe_present, &pipe_index); 
        checkRedirection(args, &in_redirect, &out_redirect, &in_file, &out_file);


        // execute the command 
        pid_t pid = fork();
        if (pid == 0) {
            
            // handle pipe
            if (pipe_present) {
                handlePipe(args, pipe_index);
            }

            // handle input redirection 
            if (in_redirect || out_redirect) {
                handleRedirection(in_redirect, out_redirect, in_file, out_file);
            }

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            wait(NULL);
        }
    }

    return 0;
}
