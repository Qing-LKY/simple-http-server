#ifndef OPT_H
#define OPT_H

#include "ds.h"
#include <stdio.h>

extern conf_para final_conf;

int parse_opt_all(int argc, char *argv[]);
void display_help(const char *arg0);
int cmdopt_parse(int argc, char *argv[], conf_para *cfg);
int copy_string(char *cnf, const char *arg);
int copy_number(int *cnf, const char *arg);
int open_conf(const char *path, FILE **fd);
int line_convert(char *s, char **left, char **right);
int line_parse(const char *le, const char *ri, conf_para *cfg);
int fileopt_parse(const char *path, conf_para *cfg);

#endif