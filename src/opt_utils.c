#include "ds.h"
#include "opt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int copy_string(char *cnf, const char *arg) {
    int len = strlen(arg);
    // too long file/directoty name
    if (len >= MAXLEN) return 1;
    // copy string
    memcpy(cnf, arg, len + 1);
    return 0;
}

int copy_number(int *cnf, const char *arg) {
    char *endptr;
    errno = 0;
    // set 0 to auto choose 10, 8 or 16
    *cnf = strtol(arg, &endptr, 0);
    // something wrong when convert
    if (errno != 0 || *endptr != 0) return 2;
    return 0;
}

int open_conf(const char *path, FILE **fd) {
    *fd = fopen(path, "r");
    if (*fd == NULL) return perror(path), 1;
    return 0;
}