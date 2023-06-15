#ifndef CONFIG_H
#define CONFIG_H

#define MAXLEN 125
#define FILEOPT_NUM 6 

// Module: option parser

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

#endif