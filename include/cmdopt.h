#ifndef CMDOPT_H
#define CMDOPT_H

#include "ds.h"

void display_help(const char *arg0);
int cmdopt_parse(int argc, char *argv[], conf_para *cfg);

#endif