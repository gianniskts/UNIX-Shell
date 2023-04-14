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
#include "./include/signals.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command 

// global variable to store the PID of the running process. Its externed in signals.h
pid_t running_pid;

#define MAX_HISTORY 100
char history[MAX_HISTORY][MAX_LINE];
int history_count = 0;

int main(void) {

    char* tokens[MAX_LINE / 2 + 1]; // command line arguments to be tokenized

    char* aliases[MAX_LINE / 2 + 1][2]; // aliases to be stored
    int alias_count = 0; // number of aliases currently stored

    // set up the signal handler for SIGINT and SIGTSTP
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    
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

        // add the command to the history
    if (strlen(users_command) > 1) {
        strncpy(history[history_count % MAX_HISTORY], users_command, MAX_LINE);
        history_count++;
    }

    // handle up and down arrow keys
    if (users_command[0] == '\033' && users_command[1] == '[') {
        if (users_command[2] == 'A') { // up arrow key
            cout << "up" << endl;
            if (history_count > 0) {
                history_count--;
                strncpy(users_command, history[history_count % MAX_HISTORY], MAX_LINE);
                parseCommand(tokens, users_command);
            }
        } else if (users_command[2] == 'B') { // down arrow key
            if (history_count < MAX_HISTORY) {
                history_count++;
                if (history_count < MAX_HISTORY) {
                    strncpy(users_command, history[history_count % MAX_HISTORY], MAX_LINE);
                } else {
                    users_command[0] = '\0';
                }
                parseCommand(tokens, users_command);
            }
        }
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
                running_pid = pid;

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
