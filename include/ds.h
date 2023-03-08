#ifndef CONFIG_H
#define CONFIG_H

#define MAXLEN 125

typedef struct {
    char CGIRoot[MAXLEN];
    char DefaultFile[MAXLEN];
    char ConfigFile[MAXLEN];
    char DocumentRoot[MAXLEN];
    int ListenPort;
    int MaxClient;
    int TimeOut;
} conf_para;

#endif