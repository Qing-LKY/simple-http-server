#include "ds.h"
#include "worker.h"
#include "sock.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int stop_all = 0;

void sig_handler(int signo) {
    stop_all = 1;
}

int set_sighandler() {
    printf(PR_BLUE "Setting signal handler...\n" PR_END);
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("Set SIGINT handler");
        return -1;
    }
    return 0;
}