#ifndef CONFIG_H
#define CONFIG_H

#define PR_BLUE "\033[34m"
#define PR_RED "\033[31m"
#define PR_END "\033[0m"

// Module: option parser

#define MAXLEN 125
#define FILEOPT_NUM 6 

typedef struct {
    char CGIRoot[MAXLEN];
    char DefaultFile[MAXLEN];
    char ConfigFile[MAXLEN];
    char DocumentRoot[MAXLEN];
    int ListenPort;
    int MaxClient;
    int TimeOut;
} conf_para;

extern char *opt_names[];
extern char short_names[];

int display_para(conf_para *cfg);
char match_name(const char *s);
void blank_para(conf_para *cfg);
void update_para(conf_para *cfg, conf_para *upd);

// Module: multithread worker

#include <pthread.h>

#define K 1024

struct _worker_ctl;

typedef struct _conn_info {
    int cli_s;
    int timeout;
    
    char req_buf[K << 4];
    char rsp_buf[K << 4];

} conn_info;

typedef struct _worker_ctl {
    // worker control info
    int status, stop;
    pthread_t tid;
    pthread_mutex_t mutex;

    // current connection
    conn_info conn;

} worker_ctl;

#define STATUS_NONE 0
#define STATUS_FREE 1
#define STATUS_BUSY 2

#endif