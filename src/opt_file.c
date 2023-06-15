#include "ds.h"
#include "opt_utils.h"
#include "opt_file.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define BUF_SIZE 1024

int line_convert(char *s, char **left, char **right) {
    int len = strlen(s), i;
    int equv = -1, fir = -1;
    *left = *right = NULL;
    for (i = 0; i < len; i++) {
        if (s[i] == '#') {
            s[len = i] = 0;
            break;
        }
        if (s[i] == '=' && equv == -1) equv = i;
        if (fir == -1 && !isspace(s[i])) fir = i;
    }
    if (equv == -1 && fir == -1) return /* empty line */ 0;
    if (equv == -1) return /* no '=' */ 1;
    if (equv == fir) return /* no left value */ 1;
    *left = s + fir;
    for (i = equv - 1; i >= fir; i--) {
        if (!isspace(s[i])) {
            s[i + 1] = 0;
            break;
        }
    }
    fir = -1;
    for (i = equv + 1; i < len; i++) {
        if (!isspace(s[i])) {
            fir = i;
            break;
        }
    }
    if (fir == -1) return /* no right value */ 1;
    if (*(s + fir) == '\"') fir++;
    *right = s + fir;
    for (i = len - 1; i >= fir; i--) {
        if (!isspace(s[i])) {
            if (s[i] == '\"') s[i] = 0;
            else s[i + 1] = 0;
            break;
        }
    }
    return 0;
}

int line_parse(const char *le, const char *ri, conf_para *cfg) {
    int err = 0, idx;
    char c = match_name(le);
    switch (c) {
        case 'c': {
            idx = 0;
            err = copy_string(cfg->CGIRoot, ri);
            break;
        }
        case 'd': {
            idx = 1;
            err = copy_string(cfg->DefaultFile, ri);
            break;
        }
        case 'o': {
            idx = 2;
            err = copy_string(cfg->DocumentRoot, ri);
            break;
        }
        case 'l': {
            idx = 3;
            err = copy_number(&cfg->ListenPort, ri);
            break;
        }
        case 'm': {
            idx = 4;
            err = copy_number(&cfg->MaxClient, ri);
            break;
        }
        case 't': {
            idx = 5;
            err = copy_number(&cfg->TimeOut, ri);
            break;
        }
        case '?': {
            printf("Unsupported left type: %s\n", le);
            return 1;
        }
        default: {
            puts("Unknown left type in parsing fileopt. Needs debug.");
            return 1;
        }
    }
    if (err == 1) {
        printf("Too long name string for %s.\n", opt_names[idx]);
        return 1;
    } else if (err == 2) {
        printf("Invaild number for %s.\n", opt_names[idx]);
        return 1;
    }
    return 0;
}

int fileopt_parse(const char *path, conf_para *cfg) {
    FILE *fd;
    char buf[BUF_SIZE], *tmp, *le, *ri;
    int line_no = 0, err = 0;
    if (open_conf(path, &fd)) {
        puts("Failed to open config file.");
        return 1;
    }
    errno = 0;
    while (1) {
        ++line_no;
        tmp = fgets(buf, BUF_SIZE, fd);
        if (errno) {
            sprintf(buf, "Line %d", line_no);
            perror(buf);
            goto bad_config;
        }
        if (tmp == NULL) break;
        err = line_convert(buf, &le, &ri);
        if (err) {
            printf("Invalid syntax.\n");
            goto bad_config;
        }
        // empty line
        if (le == NULL && ri == NULL) continue;
        err = line_parse(le, ri, cfg);
        if (err) goto bad_config;
    }
    return fclose(fd), 0;
bad_config:
    return fclose(fd), line_no;
}