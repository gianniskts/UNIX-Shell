#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/signals.h"

using namespace std;

// signal handler for SIGINT (sent by control-c)
void sigint_handler(int signum) {
    if (running_pid != 0) { // if there is a running process
        cout << endl;
        kill(running_pid, SIGINT); // kill the running process
        int status; 
        waitpid(running_pid, &status, 0); 
        running_pid = 0; 
    } 
}

// signal handler for SIGTSTP (sent by control-z)
void sigtstp_handler(int signum) { 
    if (running_pid != 0) { // if there is a running process
        cout << endl;
        kill(running_pid, SIGTSTP); // kill the running process
        int status;
        waitpid(running_pid, &status, 0);
        running_pid = 0;
    }
}