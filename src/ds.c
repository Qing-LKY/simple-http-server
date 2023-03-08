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
    printf("CGIRoot: %s\n", cfg->CGIRoot);
    printf("DefaultFile: %s\n", cfg->DefaultFile);
    printf("ConfigFile: %s\n", cfg->ConfigFile);
    printf("DocumentRoot: %s\n", cfg->DocumentRoot);
    printf("ListenPort: %d\n", cfg->ListenPort);
    printf("MaxClient: %d\n", cfg->MaxClient);
    printf("TimeOut: %d\n", cfg->TimeOut);
    return 0;
}

char match_name(const char *s) {
    int i = 0, n = FILEOPT_NUM;
    for (i = 0; i < n; i++) {
        if (strcmp(opt_names[i], s) == 0) return short_names[i];
    }
    return '?';
}