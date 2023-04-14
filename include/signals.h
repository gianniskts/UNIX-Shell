#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <unistd.h>

// declare the global variable defined in main.cpp
extern pid_t running_pid;

// signal handler for SIGINT (sent by control-c)
void sigint_handler(int signum);

// signal handler for SIGTSTP (sent by control-z)
void sigtstp_handler(int signum);

#endif
