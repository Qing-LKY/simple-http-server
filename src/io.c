#include "ds.h"
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
    if (*endptr != 0 || errno != 0) return 2;
    return 0;
}

int display_para(conf_para *cfg) {
    printf("CGIRoot: %s\n", cfg->CGIRoot);
    printf("DefaultFile: %s\n", cfg->DefaultFile);
    printf("ConfigFile: %s\n", cfg->ConfigFile);
    printf("DocumentRoot: %s\n", cfg->DocumentRoot);
    printf("ListenPort: %d\n", cfg->ListenPort);
    printf("MaxClient: %d\n", cfg->MaxClient);
    printf("TimeOut: %d\n", cfg->TimeOut);
    return 0;
}