#ifndef HTTP_H
#define HTTP_H

#include "ds.h"

extern const char methods[][10];

void parse_request(worker_ctl *ctl);
void handle_request(worker_ctl *ctl);
void url_decode(char *url);
char **form2argv(char *path, char *form, int *p_cnt);
const char *err2str(int s);
int stream2file(int id, int fd);

#endif