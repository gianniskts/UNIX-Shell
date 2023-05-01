#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <glob.h> // glob() and globfree() functions

// #include "./include/utils.h"
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

void parseCommand(char** tokens, char* command, char** next_command, bool* next_command_flag) {
    char* token = strtok(command, " \n"); // tokenize the command string

    int i = 0;
    while (token != NULL) { // loop through each token in the command
        glob_t paths; // a glob_t structure to store the matched paths
        int flags = 0;

        if (strchr(token, '*') != NULL || strchr(token, '?') != NULL) { // check if the token contains wildcards
            flags |= GLOB_TILDE; // set the GLOB_TILDE flag to expand tilde characters
            glob(token, flags, NULL, &paths); // expand the wildcard pattern using glob

            // loop through the matching paths and add them to the tokens array
            for (int j = 0; j < paths.gl_pathc; j++) {
                tokens[i++] = strdup(paths.gl_pathv[j]);
            }

            globfree(&paths); // free the memory allocated by glob
        } else if (strcmp(token, ";") == 0) {
            tokens[i] = NULL; // set the last token of the current command to NULL
            i = 0; // reset the token counter for the next command
            *next_command_flag = true; // set the flag to indicate that a next command exists

            // copy the remaining tokens to the next_command array
            while ((token = strtok(NULL, " \n")) != NULL) {
                if (strcmp(token, ";") == 0) {
                    i--; // don't add the semicolon to the next_command array
                    break; // stop copying tokens if another semicolon is found
                }
                next_command[i++] = strdup(token);
            }
            next_command[i] = NULL; // set the last element of next_command to NULL
        } else {
            tokens[i++] = strdup(token); // add the token to the tokens array as it is
        }

        token = strtok(NULL, " \n"); // move to the next token
    }

    tokens[i] = NULL; // add a NULL terminator to the end of the tokens array
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
        char* next_command[MAX_LINE]; // buffer to store the next command if the user entered a semicolon
        bool next_command_flag = false; // flag to check if the user entered a semicolon
        parseCommand(tokens, users_command, next_command, &next_command_flag);                // parse the command and store the tokens in tokens array

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

                    // print tokens and next_command
                    for (int i = 0; tokens[i] != NULL; i++) {
                        cout << tokens[i] << " ";
                    }

                    cout << endl;

                    for (int i = 0; next_command[i] != NULL; i++) {
                        cout << next_command[i] << " ";
                    }
                    

                    if (next_command_flag) {
                        execvp(tokens[0], tokens); // execute the first command

                        pid_t pid2 = fork(); // fork another child process
                        if (pid2 == 0) {
                            execvp(next_command[0], next_command); // execute the next command
                            perror("execvp");
                            exit(1);
                        } else {
                            wait(NULL); // wait for the child process to finish
                        }
                        // execvp(next_command[0], next_command); // execute the next command
                        // perror("execvp");
                        // exit(1);
                    } else {
                        execvp(tokens[0], tokens); // execute the command
                        perror("execvp");
                        exit(1);
                    }

                } else {
                    wait(NULL); // wait for the child process to finish
                }
            }
        }
    }
    return 0;
}
