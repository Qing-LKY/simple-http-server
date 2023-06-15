#include "ds.h"
#include "utils.h"
#include "opt_cmd.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

void display_help(const char *arg0) {
    printf("Usage: %s [OPTION]...\n", arg0);
    puts("Emulate to boot SHTTPD server (with custom options).");
    puts("Use for testing implementation of configuring.");
    puts("");
    puts("Configure:");
    puts("  -c DIRECTORY, --CGIRoot=DIRECTORY");
    puts("  -d FILE, --DefaultFile=FILE");
    puts("  -f FILE, --ConfigFile=FILE");
    puts("  -o DIRECTORY, --DocumentRoot=DIRECTORY");
    puts("  -l NUM, --ListenPort=NUM");
    puts("  -m NUM, --MaxClient=NUM");
    puts("  -t NUM, --TimeOut=NUM");
    puts("");
    puts("Show this message and exit:");
    puts("  -h, --Help");
    puts("");
    puts("Repository page: <https://github.com/Qing-LKY/SHTTPD-Config-Analyzer>");
    puts("Read README.md for more infomation.");
    exit(EXIT_SUCCESS);
}

int cmdopt_parse(int argc, char *argv[], conf_para *cfg) {
    static struct option opts[] = {
        {"CGIRoot", required_argument, 0, 'c'},
        {"DefaultFile", required_argument, 0, 'd'},
        {"ConfigFile", required_argument, 0, 'f'},
        {"DocumentRoot", required_argument, 0, 'o'},
        {"ListenPort", required_argument, 0, 'l'},
        {"MaxClient", required_argument, 0, 'm'},
        {"TimeOut", required_argument, 0, 't'},
        {"Help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    static const char optstring[] = "c:d:f:o:l:m:t:h";
    while (1) {
        int optidx, err = 0;
        int c = getopt_long(argc, argv, optstring, opts, &optidx);
        // parse finished
        if (c == -1) break;
        switch (c) {
            case 'h': display_help(argv[0]); break;
            case 'c': {
                optidx = 0;
                err = copy_string(cfg->CGIRoot, optarg);
                break;
            }
            case 'd': {
                optidx = 1;
                err = copy_string(cfg->DefaultFile, optarg);
                break;
            }
            case 'f': {
                optidx = 2;
                err = copy_string(cfg->ConfigFile, optarg);
                break;
            }
            case 'o': {
                optidx = 3;
                err = copy_string(cfg->DocumentRoot, optarg);
                break;
            }
            case 'l': {
                optidx = 4;
                err = copy_number(&cfg->ListenPort, optarg);
                break;
            }
            case 'm': {
                optidx = 5;
                err = copy_number(&cfg->MaxClient, optarg);
                break;
            }
            case 't': {
                optidx = 6;
                err = copy_number(&cfg->TimeOut, optarg);
                break;
            }
            case '?': return 1;
            default: {
                puts("Unknown error in parsing commands. Needs debug.");
                return 1;
            }
        }
        if (err == 1) {
            printf("Too long name string for %s.\n", opts[optidx].name);
            return 1;
        } else if (err == 2) {
            printf("Invaild number for %s.\n", opts[optidx].name);
            return 1;
        }
    }
    return 0;
}