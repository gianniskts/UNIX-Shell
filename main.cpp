// #include <iostream>
// #include <cstring>
// #include <unistd.h>   // for fork(), execvp()
// #include <sys/wait.h> // for wait()
// #include <fcntl.h>    // for open()

// #include "utils.h"

// using namespace std;

// #define MAX_INPUT_LENGTH 1024
// #define MAX_TOKENS 64

// int main()
// {
//     char input[MAX_INPUT_LENGTH];
//     char* tokens[MAX_TOKENS];
//     while (true)
//     {
//         cout << "in-mysh-now:>";
//         cin.getline(input, MAX_INPUT_LENGTH);
//         tokenizeInput(input, tokens);
//         if (tokens[0] == NULL)
//             continue; // user entered nothing
//         if (strcmp(tokens[0], "exit") == 0)
//             break; // user entered "exit", so we exit the shell

//         Command command;
//         parseCommand(tokens, MAX_TOKENS, command);
//         executeCommand(command);
//     }
//     return 0;
// }

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "utils.h"
#include "redirection.h"

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
        int j = 0;
        while (tokens[j] != NULL) {
            if (strcmp(tokens[j], "|") == 0) {
                has_pipe = 1;
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
                    int redirect_input = 0;
                    int redirect_output = 0;
                    char* input_file = NULL;
                    char* output_file = NULL;
                    int append_output = 0;
                    int j = 0;
                    while (args2[j] != NULL) {
                        if (strcmp(args2[j], "<") == 0) {
                            redirect_input = 1;
                            input_file = args2[j + 1];
                            args2[j] = NULL;
                        } else if (strcmp(args2[j], ">") == 0) {
                            redirect_output = 1;
                            output_file = args2[j + 1];
                            args2[j] = NULL;
                        } else if (strcmp(args2[j], ">>") == 0) {
                            redirect_output = 1;
                            output_file = args2[j + 1];
                            append_output = 1;
                            args2[j] = NULL;
                        }
                        j++;
                    }
                     if (redirect_input) {
                        // redirect input from a file
                        int fd = open(input_file, O_RDONLY);
                        if (fd == -1) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    if (redirect_output) {
                        // redirect output to a file
                        int fd;
                        if (append_output) {
                            fd = open(output_file, O_CREAT | O_WRONLY | O_APPEND, 0666);
                        } else {
                            fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                        }
                        if (fd == -1) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }


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
                // child process
                if (redirect_input) {
                    // redirect input from a file
                    int fd = open(input_file, O_RDONLY);
                    if (fd == -1) {
                        perror("open");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                if (redirect_output) {
                    // redirect output to a file
                    int fd;
                    if (append_output) {
                        fd = open(output_file, O_CREAT | O_WRONLY | O_APPEND, 0666);
                    } else {
                        fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    }
                    if (fd == -1) {
                        perror("open");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
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
