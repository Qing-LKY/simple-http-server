#include "ds.h"
#include "utils.h"
#include "cmdopt.h"
#include "fileopt.h"
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

int main(int argc, char *argv[]) {
     if (parse_cmd(argc, argv)) return 1;
     if (parse_file()) return 1;
     merge_para();
     return 0;
}