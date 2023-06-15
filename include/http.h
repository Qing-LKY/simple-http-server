#ifndef HTTP_H
#define HTTP_H

#include "ds.h"

extern const char methods[][10];

void parse_request(worker_ctl *ctl);
void handle_request(worker_ctl *ctl);
void url_decode(char *url);

#endif