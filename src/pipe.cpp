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
        while (tokens[j] != NULL) {
            if (strcmp(tokens[j], "|") == 0) {
                *has_pipe = true;
                tokens[j] = NULL;
                // create a new array for the second subcommand
                char* args2[MAX_LINE / 2 + 1];
                int k = j + 1;
                int l = 0;
                while (tokens[k] != NULL) {
                    args2[l++] = tokens[k++];
                }
                args2[l] = NULL;
 
                // execute the two subcommands in a pipeline
                int pipefd[2];
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(1);
                }
                pid_t pid1 = fork();
                if (pid1 == 0) {
                    // child process 1
                    close(pipefd[0]); // close the read end of the pipe
                    dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to the write end of the pipe
                    execvp(tokens[0], tokens);
                    perror("execvp");
                    exit(1);
                }
                pid_t pid2 = fork();
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