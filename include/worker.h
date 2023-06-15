#ifndef WORKER_H
#define WORKER_H

#include "ds.h"

extern worker_ctl *workers;

int init_workers();
void destory_workers();

void main_loop();

#endif