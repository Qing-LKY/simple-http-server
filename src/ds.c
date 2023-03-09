#include "ds.h"
#include <stdio.h>
#include <string.h>

char *opt_names[] = {
    "CGIRoot",
    "DefaultFile",
    "DocumentRoot",
    "ListenPort",
    "MaxClient",
    "TimeOut"  
};

char short_names[] = "cdolmt";

int display_para(conf_para *cfg) {
    if (*cfg->CGIRoot) printf("CGIRoot: %s\n", cfg->CGIRoot);
    if (*cfg->DefaultFile) printf("DefaultFile: %s\n", cfg->DefaultFile);
    if (*cfg->ConfigFile) printf("ConfigFile: %s\n", cfg->ConfigFile);
    if (*cfg->DocumentRoot) printf("DocumentRoot: %s\n", cfg->DocumentRoot);
    if (cfg->ListenPort != -1) printf("ListenPort: %d\n", cfg->ListenPort);
    if (cfg->MaxClient != -1) printf("MaxClient: %d\n", cfg->MaxClient);
    if (cfg->TimeOut != -1) printf("TimeOut: %d\n", cfg->TimeOut);
    return 0;
}

char match_name(const char *s) {
    int i = 0, n = FILEOPT_NUM;
    for (i = 0; i < n; i++) {
        if (strcmp(opt_names[i], s) == 0) return short_names[i];
    }
    return '?';
}

void blank_para(conf_para *cfg) {
    *cfg->CGIRoot = 0;
    *cfg->ConfigFile = 0;
    *cfg->DefaultFile = 0;
    *cfg->DocumentRoot = 0;
    cfg->ListenPort = -1;
    cfg->MaxClient = -1;
    cfg->TimeOut = -1;
}

void update_para(conf_para *cfg, conf_para *upd) {
    if (*upd->CGIRoot) strcpy(cfg->CGIRoot, upd->CGIRoot);
    if (*upd->ConfigFile) strcpy(cfg->ConfigFile, upd->ConfigFile);
    if (*upd->DefaultFile) strcpy(cfg->DefaultFile, upd->DefaultFile);
    if (*upd->DocumentRoot) strcpy(cfg->DocumentRoot, upd->DocumentRoot);
    if (upd->ListenPort != -1) cfg->ListenPort = upd->ListenPort;
    if (upd->MaxClient != -1) cfg->MaxClient = upd->MaxClient;
    if (upd->TimeOut != -1) cfg->TimeOut = upd->TimeOut;
}
