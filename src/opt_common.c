#include "ds.h"
#include "opt_utils.h"
#include "opt_cmd.h"
#include "opt_file.h"
#include <stdio.h>

conf_para def_conf = {
    .CGIRoot = "/usr/local/var/www/cgi-bin/",
    .DefaultFile = "index.html",
    .DocumentRoot = "/usr/local/var/www/",
    .ConfigFile = "/etc/emu-shttpd.conf",
    .ListenPort = 8080,
    .MaxClient = 4,
    .TimeOut = 3,
};

conf_para cmd_conf = { "", "", "", "", -1, -1, -1 };
conf_para file_conf = { "", "", "", "", -1, -1, -1 };
conf_para final_conf = { "", "", "", "", -1, -1, -1 };

int parse_cmd(int argc, char *argv[]) {
    conf_para *cfg = &cmd_conf;
    int err = cmdopt_parse(argc, argv, cfg);
    if (err) {
        puts("Failed in command option parsing.");
        return 1;
    }
    puts("Options gained in command:");
    display_para(cfg);
    puts("");
    return 0;
}

int parse_file() {
    conf_para *cfg = &file_conf;
    char *path = *cmd_conf.ConfigFile ? cmd_conf.ConfigFile : def_conf.ConfigFile;
    int err = fileopt_parse(path, cfg);
    if (err) {
        printf("Failed in config file parsing (line %d).\n", err);
        return 1;
    }
    puts("Options gained in file:");
    display_para(cfg);
    puts("");
    return 0;
}

int merge_para() {
    update_para(&final_conf, &def_conf);
    update_para(&final_conf, &file_conf);
    update_para(&final_conf, &cmd_conf);
    puts("Options after merge:");
    display_para(&final_conf);
    puts("");
    return 0;
}

int test_para() {
    if (final_conf.MaxClient >= 10) {
        fprintf(stderr, "Too large MaxClient! It should below 10!\n");
        return 1;
    }
    return 0;
}

int parse_opt_all(int argc, char *argv[]) {
    printf(PR_BLUE "Parsing options...\n" PR_END);
    if (parse_cmd(argc, argv)) return 1;
    if (parse_file()) return 1;
    merge_para();
    if (test_para()) return 1;
    return 0;
}