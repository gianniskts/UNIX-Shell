#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "./include/utils.h"
#include "./include/redirection.h"
#include "./include/pipe.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command 

int main(void) {

    char* tokens[MAX_LINE / 2 + 1]; // command line arguments to be tokenized
    int should_run = 1;           // flag to determine when to exit program

    while (should_run) {
        printf("in-mysh-now:>");
        fflush(stdout);

        // read input from the user 
        char users_command[MAX_LINE];
        fgets(users_command, MAX_LINE, stdin);

        parseCommand(tokens, users_command);

        // check for pipes in the command
        int has_pipe = 0;
        handlePipe(tokens, &has_pipe);
        
        // if theres no pipe, check for input/output redirection
        if (!has_pipe) {
            // check for input/output redirection
            int redirect_input = 0;
            int redirect_output = 0;
            char* input_file = NULL;
            char* output_file = NULL;
            int append_output = 0;
           
            checkRedirection(tokens, &redirect_input, &redirect_output, &input_file, &output_file, &append_output);

            // execute the command with input/output redirection
            pid_t pid = fork();
            if (pid == 0) {
                if (redirect_input || redirect_output) {
                    handleRedirection(redirect_input, redirect_output, input_file, output_file, append_output);
                }
                
                execvp(tokens[0], tokens);
                perror("execvp");
                exit(1);
            } else {
                wait(NULL);
            }
        }
    }

    return 0;
}
