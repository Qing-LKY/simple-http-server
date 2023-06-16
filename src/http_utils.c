#include "http.h"
#include "ds.h"
#include "opt.h"
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

char **form2argv(char *path, char *form, int *p_cnt) {
    // convert x=xx&y=yy&... to argv[]
    char *p = form;
    int i = 1, cnt = 1;
    while (*p != 0) {
        if (*p == '&') *p = 0, ++cnt;
        p++;
    }
    char **argv = (char **)calloc(sizeof(char *), cnt + 2);
    argv[0] = path;
    p = form;
    argv[1] = p;
    while (i < cnt) {
        if (*p == 0) argv[++i] = p + 1;
        p++;
    }
    argv[cnt + 1] = NULL;
    for (i = 1; i <= cnt; i++) url_decode(argv[i]);
    *p_cnt = cnt;
    return argv;
}

static inline int ishexval(char c) {
    return isdigit(c) || isalpha(c);
}

static inline int gethexval(char c) {
    if (islower(c)) return c - 'a' + 10;
    if (isupper(c)) return c - 'A' + 10;
    if (isdigit(c)) return c - '0';
    return 0;
}

static inline int gethex(char *p) {
    return (gethexval(*(p + 1)) << 4) | (gethexval(*(p + 2)));
}

void url_decode(char *url) {
    if (url == NULL) return;
    char *p = url, *q = url;
    while (*q != 0) {
        if (*q == '%' && ishexval(*(q + 1)) && ishexval(*(q + 2))) {
            *p = gethex(q);
            q += 3;
            p += 1;
        } else {
            *p = *q;
            ++p; ++q;
        }
    }
    *p = 0;
}

const char HTTP_STATUS[][20] = {
    "OK", "NOT_FOUND", "UNSUPPORT", "UNKNOWN",
    "URL_TOO_LARGE", "SERVICE_FAILED"
};

const char *err2str(int s) {
    switch (s) {
        case HTTP_OK: return HTTP_STATUS[0];
        case HTTP_NOT_FOUND: return HTTP_STATUS[1];
        case HTTP_BAD_REQUEST: return HTTP_STATUS[2];
        case HTTP_URL_TOO_LARGE: return HTTP_STATUS[4];
        case HTTP_SERVICE_FAILED: return HTTP_STATUS[5];
        default: return HTTP_STATUS[3];
    }
    return NULL;
}

int stream2file(int id, int fd) {
    // 从 fd 中读出所有数据并写入临时文件中
    // 返回临时文件的描述符
    int n = 0;
    char name[200], buf[500];
    sprintf(name, "%s/worker%d.tmp", final_conf.CGIRoot, id);
    int nfd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (nfd == -1) goto bad2;
    while((n = read(fd, buf, 500)) > 0) {
        if (n < 0) goto bad1;
        n = write(nfd, buf, n);
        if (n < 0) goto bad1;
    }
    n = lseek(nfd, 0, SEEK_SET);
    if (n != 0) goto bad1;
    return nfd;
bad1:
    close(nfd);
bad2:
    perror("stream2file");
    return -1;
}