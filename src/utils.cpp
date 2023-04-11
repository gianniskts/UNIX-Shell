#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/utils.h"

void parseCommand(char** tokens, char* users_command) {
     // Parse the input command string into a vector of tokens (arguments).
        char* token = strtok(users_command, " \n");
        int i = 0;
        while (token != NULL) {
            tokens[i++] = token;
            token = strtok(NULL, " \n");
        }
        tokens[i] = NULL;
} 