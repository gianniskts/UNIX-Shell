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
#include "./include/history.h"

using namespace std;

#define MAX_LINE 80     // The maximum length command 
#define HISTORY_SIZE 20 // Τhe maximum number of commands to store in history

char* history[HISTORY_SIZE]; // array to store previous commands
int history_index = 0;       // current index in the history array

pid_t running_pid; // global variable to store the PID of the running process. Its externed in signals.h

int main(void) {

    char* tokens[MAX_LINE / 2 + 1]; // command line arguments to be tokenized

    char* aliases[MAX_LINE / 2 + 1][2]; // aliases to be stored
    int alias_count = 0; // number of aliases currently stored

    // set up the signal handler for SIGINT and SIGTSTP
    signal(SIGINT, sigint_handler);   // ctrl-c
    signal(SIGTSTP, sigtstp_handler); // ctrl-z
    
    while (true) {
        printf("in-mysh-now:>"); // print the prompt 
        fflush(stdout);          // flush the output buffer

        // read input from the user and parse it
        char users_command[MAX_LINE];          // buffer to store the user's command
        fgets(users_command, MAX_LINE, stdin); // read the user's command from stdin and store it in users_command
        // add command to history
        addHistory(users_command, history, &history_index); // add the command to the history array
        parseCommand(tokens, users_command);                // parse the command and store the tokens in tokens array

        if (strcmp(tokens[0], "exit") == 0)  // if the user entered exit, exit the shell
            break;
        
        if (checkHistory(tokens, history, history_index)) // check if the user entered a history command
            continue;

        if (checkAlias(tokens, aliases, alias_count)) // check if the user entered an alias command
            continue;

        // check for pipes in the command
        bool has_pipe = 0; // flag to check if there is a pipe in the command
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
