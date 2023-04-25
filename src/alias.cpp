#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/alias.h"

using namespace std;

#define MAX_LINE 80 // The maximum length command 

bool checkAlias(char* tokens[], char* aliases[][2], int& alias_count) {
    if (strcmp(tokens[0], "createalias") == 0 && tokens[1] != NULL && tokens[2] != NULL) { // if the user entered createalias <NewName> <OldName>
        aliases[alias_count][0] = strdup(tokens[1]); // add the new name to the aliases array
        aliases[alias_count][1] = strdup(tokens[2]); // add the old name to the aliases array
        alias_count++; // increment the alias count
        return true;
    } else if (strcmp(tokens[0], "destroyalias") == 0 && tokens[1] != NULL) { // if the user entered destroyalias <AliasedName>
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(aliases[i][0], tokens[1]) == 0) { // if the alias exists
                free(aliases[i][0]);  // free the memory allocated for the alias
                free(aliases[i][1]);  // free the memory allocated for the aliased command
                aliases[i][0] = NULL; // set the alias to NULL
                aliases[i][1] = NULL; // set the aliased command to NULL
                break;
            }
        }
        return true;
    }

    // check for aliases and replace tokens if necessary
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i][0] != NULL && strcmp(aliases[i][0], tokens[0]) == 0) { // if the alias exists
            tokens[0] = aliases[i][1]; // replace the alias with the aliased command
            break;
        }
    }

    return false;
}