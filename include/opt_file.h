#ifndef FILEOPT_H
#define FILEOPT_H

#include "ds.h"

int line_convert(char *s, char **left, char **right);
int line_parse(const char *le, const char *ri, conf_para *cfg);
int fileopt_parse(const char *path, conf_para *cfg);

#endif