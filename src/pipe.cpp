#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "redirection.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command 


void handlePipe(char** tokens, bool* has_pipe) {
    int j = 0;
    while (tokens[j] != NULL) { // iterate through the tokens to find the pipe symbol
        if (strcmp(tokens[j], "|") == 0) {
            *has_pipe = true;
            tokens[j] = NULL; // set the token to NULL so that execvp() doesn't try to execute it
            // create a new array for the second subcommand
            char* args2[MAX_LINE / 2 + 1]; // the maximum number of arguments for the second subcommand is half of the maximum number of arguments for the entire command
            int k = j + 1; // the index of the first token of the second subcommand
            int l = 0; // the index of the current token of the second subcommand
            while (tokens[k] != NULL) { // copy the tokens of the second subcommand into the new array
                args2[l++] = tokens[k++];
            }
            args2[l] = NULL; // set the last token to NULL so that execvp() doesn't try to execute it

            // execute the two subcommands in a pipeline
            int pipefd[2]; // the file descriptors for the read and write ends of the pipe
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
            pid_t pid1 = fork(); // create the first child process
            if (pid1 == 0) {
                // child process 1
                close(pipefd[0]); // close the read end of the pipe
                dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to the write end of the pipe
                execvp(tokens[0], tokens); // execute the first subcommand
                perror("execvp");
                exit(1);
            }
            pid_t pid2 = fork(); // create the second child process
            if (pid2 == 0) {
                // child process 2
                close(pipefd[1]); // close the write end of the pipe
                dup2(pipefd[0], STDIN_FILENO); // redirect stdin to the read end of the pipe
                // check for input/output redirection
                bool redirect_input = 0;
                bool redirect_output = 0;
                bool append_output = 0;
                char* input_file = NULL;
                char* output_file = NULL;
                
                checkRedirection(args2, &redirect_input, &redirect_output, &input_file, &output_file, &append_output);
                if (redirect_input || redirect_output)
                    handleRedirection(redirect_input, redirect_output, input_file, output_file, append_output);
                execvp(args2[0], args2);
                perror("execvp");
                exit(1);
            }
            // parent process
            close(pipefd[0]); // close both ends of the pipe
            close(pipefd[1]);
            waitpid(pid1, NULL, 0); // wait for both child processes to finish
            waitpid(pid2, NULL, 0);
            break; // exit the loop since we've already executed the command
        }
            j++;
        }
}