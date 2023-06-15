#include "worker.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

void sig_handler(int signo) {
    destory_workers();
    exit(EXIT_SUCCESS);
}

int set_sighandler() {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("Set SIGINT handler");
        return -1;
    }
    return 0;
}