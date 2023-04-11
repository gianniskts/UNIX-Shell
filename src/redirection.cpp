#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/redirection.h"

using namespace std;

void checkRedirection(char** tokens, int* redirect_input, int* redirect_output, char** input_file, char** output_file, int* append_output) {
    int j = 0;
    while (tokens[j] != NULL) {
        if (strcmp(tokens[j], "<") == 0) {
            *redirect_input = 1;
            *input_file = tokens[j + 1];
            tokens[j] = NULL;
        } else if (strcmp(tokens[j], ">") == 0) {
            *redirect_output = 1;
            *output_file = tokens[j + 1];
            tokens[j] = NULL;
        } else if (strcmp(tokens[j], ">>") == 0) {
            *redirect_output = 1;
            *output_file = tokens[j + 1];
            *append_output = 1;
            tokens[j] = NULL;
        }
        j++;
    }
}