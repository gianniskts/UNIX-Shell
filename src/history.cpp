#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../include/history.h"
#include "../include/utils.h"

#define HISTORY_SIZE 20

using namespace std;

void addHistory(char* command, char* history[], int* history_index) {
    history[*history_index % HISTORY_SIZE] = strdup(command);
    (*history_index)++;
}

bool checkHistory(char* tokens[], char* history[], int history_index) {
    // check for history command
    if (strcmp(tokens[0], "history") == 0) {
        // print previous commands with indices
        for (int i = 0; i < history_index; i++) {
            cout << i+1 << ": " << history[i] << endl;
        }
        return true;
    }

    // execute command from history
    if (tokens[0][0] == 'h') {
        // execute command with index from history
        int index = atoi(tokens[0]+1);
        if (index <= 0 || index > history_index) {
            printf("Invalid index.\n");
            return true;
        }
        char* command = strdup(history[(index-1) % HISTORY_SIZE]);
        // printf("%s", command);
        parseCommand(tokens, command);
        free(command);
    }

    return false;
}

