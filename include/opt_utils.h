#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

int copy_string(char *cnf, const char *arg);
int copy_number(int *cnf, const char *arg);
int open_conf(const char *path, FILE **fd);

#endif