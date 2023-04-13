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
#include "./include/background.h"
#include "./include/alias.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command 

void sigintHandler(int sig) {
    // do nothing, just print a message
    cout << "\nCaught SIGINT signal\n";
}

void sigtstpHandler(int sig) {
    // send the signal to the running program
    kill(0, sig);
}


int main(void) {

    char* tokens[MAX_LINE / 2 + 1]; // command line arguments to be tokenized

    char* aliases[MAX_LINE / 2 + 1][2]; // aliases to be stored
    int alias_count = 0; // number of aliases currently stored

    // register signal handlers
    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);

    while (true) {
        printf("in-mysh-now:>");
        fflush(stdout);

        // read input from the user and parse it
        char users_command[MAX_LINE];
        fgets(users_command, MAX_LINE, stdin);
        parseCommand(tokens, users_command);

        // check for exit command
        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        if (checkAlias(tokens, aliases, alias_count))   
            continue;

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
           
            // check for input/output redirection
            checkRedirection(tokens, &redirect_input, &redirect_output, &input_file, &output_file, &append_output);

            // check for background process
            // TODO: check for background process in pipes etc
            pid_t bg_pid;
            bool background = checkBackground(tokens, &bg_pid);
            checkFinishedBackground(bg_pid, background);

            if (!background) {
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
    }
    return 0;
}
