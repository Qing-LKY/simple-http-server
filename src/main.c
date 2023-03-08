#include "ds.h"
#include "io.h"
#include "cmdopt.h"
#include <stdio.h>

conf_para config = {
     /* CGIRoot */ "/usr/local/var/www/cgi-bin/",
     /* DefaultFile */ "index.html",
     /* DocumentRoot */ "/usr/local/var/www/",
     /* ConfigFile */ "/etc/SHTTPD.conf",
     /* ListenPort */ 8080,
     /* MaxClient */ 4,
     /* TimeOut */ 3,
};

int main(int argc, char *argv[]) {
     conf_para *cfg = &config;
     int err = cmdopt_parse(argc, argv, cfg);
     if (err) {
          puts("Failed in command option parsing.");
          return 1;
     }
     display_para(cfg);
     return 0;
}