#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "../include/utils.h"

void parseCommand(char** tokens, char* command) {
    char* token = strtok(command, " \n"); // tokenize the command string

    int i = 0;
    while (token != NULL) { // loop through each token in the command
        glob_t paths; // create a glob_t structure to store the matched paths
        int flags = 0;

        if (strchr(token, '*') != NULL || strchr(token, '?') != NULL) { // check if the token contains wildcards
            flags |= GLOB_TILDE; // set the GLOB_TILDE flag to expand tilde characters
            glob(token, flags, NULL, &paths); // expand the wildcard pattern using glob

            // loop through the matching paths and add them to the tokens array
            for (int j = 0; j < paths.gl_pathc; j++) {
                tokens[i++] = strdup(paths.gl_pathv[j]);
            }

            globfree(&paths); // free the memory allocated by glob
        } else {
            tokens[i++] = strdup(token); // add the token to the tokens array as-is
        }

        token = strtok(NULL, " \n"); // move to the next token
    }

    tokens[i] = NULL; // add a NULL terminator to the end of the tokens array
}
