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

bool checkSemicolumn(char* users_command) {
    // checks if there is a ; in input 
    
}

int main(void) {

    char* tokens[MAX_LINE / 2 + 1]; // command line arguments to be tokenized

    char* aliases[MAX_LINE / 2 + 1][2]; // aliases to be stored
    int alias_count = 0; // number of aliases currently stored

    // set up the signal handler for SIGINT and SIGTSTP
    signal(SIGINT, sigint_handler);   // when ctrl-c it triggers the sigint_handler function 
    signal(SIGTSTP, sigtstp_handler); // when ctrl-z it triggers the sigtstp_handler function
    
    while (true) {
        cout << "in-mysh-now:>"; // print the prompt 
        fflush(stdout);          // flush the output buffer

        // read input from the user and parse it
        char users_command[MAX_LINE];          // buffer to store the user's command
        fgets(users_command, MAX_LINE, stdin); // read the user's command from stdin and store it in users_command
        // add command to history
        addHistory(users_command, history, &history_index); // add the command to the history array
        parseCommand(tokens, users_command);                // parse the command and store the tokens in tokens array

        if (strcmp(tokens[0], "exit") == 0)  // if the user entered exit, exit the shell
            break;

        if (strcmp(tokens[0], "cd") == 0) { // if the user entered cd, change the directory
            if (tokens[1] != NULL && strcmp(tokens[1], "..") == 0) { // check if the second token is ".."
                chdir(".."); // change to the parent directory
            } else if (chdir(tokens[1]) != 0) { // call chdir() with the second token (path to change to)
                perror("cd"); // if chdir() failed, print an error message
            }
            continue; // continue to the next iteration of the while loop
        }
        
        if (checkHistory(tokens, history, history_index)) // check if the user entered a history command
            continue;

        if (checkAlias(tokens, aliases, alias_count)) // check if the user entered an alias command
            continue;

        // check for pipes in the command
        bool has_pipe = 0; // flag to check if there is a pipe in the command
        handlePipe(tokens, &has_pipe); // handle pipes in the command
        
        // if theres no pipe, check for input/output redirection
        if (!has_pipe) {
            // check for input/output redirection
            bool redirect_input = 0;  // flag to check if there is input redirection <
            bool redirect_output = 0; // flag to check if there is output redirection >
            bool append_output = 0;   // flag to check if there is output redirection >>
            char* input_file = NULL;  // file to redirect input from
            char* output_file = NULL; // file to redirect output to
           
            // check for input/output redirection
            checkRedirection(tokens, &redirect_input, &redirect_output, &input_file, &output_file, &append_output);

            // check for background process
            pid_t bg_pid; // pid of the background process
            bool background = checkBackground(tokens, &bg_pid); // check if the command is a background process
            checkFinishedBackground(bg_pid, background); // check if the background process has finished

            if (!background) { 
                // execute the command with input/output redirection
                pid_t pid = fork(); 
                running_pid = pid; // store the pid of the running process

                if (pid == 0) { // child process
                    if (redirect_input || redirect_output) { // if there is input/output redirection
                        handleRedirection(redirect_input, redirect_output, input_file, output_file, append_output);
                    }

                        execvp(tokens[0], tokens); // execute the command
                        perror("execvp");
                        exit(1);
                } else {
                    wait(NULL); // wait for the child process to finish
                }
            }
        }
    }
    return 0;
}
