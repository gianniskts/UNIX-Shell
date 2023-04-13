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
    if (strcmp(tokens[0], "createalias") == 0 && tokens[1] != NULL && tokens[2] != NULL) {
        aliases[alias_count][0] = strdup(tokens[1]);
        aliases[alias_count][1] = strdup(tokens[2]);
        alias_count++;
        return true;
    } else if (strcmp(tokens[0], "destroyalias") == 0 && tokens[1] != NULL) {
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(aliases[i][0], tokens[1]) == 0) {
                free(aliases[i][0]);
                free(aliases[i][1]);
                aliases[i][0] = NULL;
                aliases[i][1] = NULL;
                break;
            }
        }
        return true;
    }

    // check for aliases and replace tokens if necessary
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i][0] != NULL && strcmp(aliases[i][0], tokens[0]) == 0) {
            tokens[0] = aliases[i][1];
            break;
        }
    }

    return false;
}