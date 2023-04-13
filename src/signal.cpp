// #include <iostream>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <fcntl.h>

// using namespace std;

// // signal handler for SIGINT (sent by control-c)
// void sigint_handler(int signum) {
//     if (running_pid != 0) {
//         kill(running_pid, SIGINT);
//         int status;
//         waitpid(running_pid, &status, 0);
//         running_pid = 0;
//     } else {
//         return; // example input ^C ls
//     }
// }

// // signal handler for SIGTSTP (sent by control-z)
// void sigtstp_handler(int signum) {
//     if (running_pid != 0) {
//         kill(running_pid, SIGTSTP);
//         suspended_pid = running_pid;
//         running_pid = 0;
//     }
// }