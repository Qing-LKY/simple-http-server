#include "ds.h"
#include "opt.h"
#include "worker.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

const char methods[][10] = {
    "GET", "POST", "UNKNOWN"
};

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

int get_line(char *s, char *e, char **nex) {
    int n = 0;
    while (s != e && *s != '\n') s++, n++;
    if (s != e) *s = 0;
    if (*(s - 1) == '\r') *(s - 1) = 0, n--;
    if (s != e) *nex = s + 1;
    else *nex = e;
    return n;
}

int deal_first_line(char *s, int n, conn_info *conn) {
    int i;
    char *p;
    while (isspace(*s) && n > 0) s++, n--;
    p = s;
    while (!isspace(*p) && n > 0) p++, n--;
    *p = 0;
    conn->method = -1;
    for (i = 0; i < SUPPORT_METHODS; i++) {
        if (strcmp(s, methods[i]) == 0) {
            conn->method = i;
            break;
        }
    }
    if (conn->method == -1) {
        conn->method = HTTP_UNKNOWN;
        goto bad;
    }
    s = p + 1; n--;
    while (isspace(*s) && n > 0) s++, n--;
    p = s;
    while (!isspace(*p) && n > 0) p++, n--;
    *p = 0;
    conn->req_url = s;
    return 0;
bad:
    conn->req_err = HTTP_BAD_REQUEST;
    return 1;
}

void try_open_file(worker_ctl *ctl) {
    printf("Thread (i=%d): Open file.\n", (int)(ctl - workers));
    conn_info *conn = &ctl->conn;
    char *file = conn->rsp_buf;
    conn->req_fd = open(file, O_RDONLY, 0644);
    if (conn->req_fd == -1) {
        perror("open file");
        conn->req_err = HTTP_NOT_FOUND;
        return;
    }
    fstat(conn->req_fd, &conn->fd_stat);
    if (S_ISDIR(conn->fd_stat.st_mode)) {
        // 如果是个目录就寻找目录下的默认文件
        close(conn->req_fd);
        int f_len = strlen(file);
        if (file[f_len - 1] == '/') file[f_len - 1] = 0;
        sprintf(file, "%s/%s", file, final_conf.DefaultFile);
        printf("Thread (i=%d): file: %s\n", (int)(ctl - workers), file);
        conn->req_fd = open(file, O_RDONLY, 0644);
        if (conn->req_fd == -1) {
            perror("open file");
            goto forbindden;
        }
        fstat(conn->req_fd, &conn->fd_stat);
        if (S_ISDIR(conn->fd_stat.st_mode)) {
            close(conn->req_fd);
            goto forbindden;
        }
    }
    return;
forbindden:
    conn->req_fd = -1;
    conn->req_err = HTTP_FORBIDDEN;
    return;
}

void check_url(worker_ctl *ctl) {
    printf("Thread (i=%d): Checking url.\n", (int)(ctl - workers));
    int mxlen, len1, len2, i;
    char *buf = ctl->conn.rsp_buf;
    mxlen = sizeof(ctl->conn.rsp_buf);
    // 排除掉后面的 ? 部分
    len2 = strlen(ctl->conn.req_url);
    ctl->conn.req_get = ctl->conn.req_url + len2;
    for (i = 0; i < len2; i++) {
        if (ctl->conn.req_url[i] == '?') {
            ctl->conn.req_url[i] = 0;
            ctl->conn.req_get = ctl->conn.req_url + i + 1;
            break;
        }
    }
    // 将 url 和根目录拼接起来
    printf("Thread (i=%d): Combine filename.\n", (int)(ctl - workers));
    len1 = strlen(final_conf.DocumentRoot);
    len2 = strlen(ctl->conn.req_url);
    if (len1 + len2 >= mxlen - 125) {
        printf("Too large url\n");
        ctl->conn.req_err = HTTP_URL_TOO_LARGE;
        return;
    }
    sprintf(buf, "%s%s", final_conf.DocumentRoot, ctl->conn.req_url);
    printf("Thread (i=%d): Try file %s.\n", (int)(ctl - workers), buf);
    buf[len1 + len2] = 0;
    // 确认目录/文件是否存在
    if (access(buf, R_OK) != 0) {
        ctl->conn.req_err = HTTP_NOT_FOUND;
        return;
    }
    // 尝试打开文件
    try_open_file(ctl);
}

void parse_request(worker_ctl *ctl) {
    printf("Thread (i=%d): Parsing header.\n", (int)(ctl - workers));
    int err, tn, n; 
    char *p, *e, *nexp;
    n = ctl->conn.req_len;
    p = ctl->conn.req_buf;
    e = p + n;
    // 处理第一行: Method url HTTP/x.x
    ctl->conn.req_err = HTTP_OK;
    tn = get_line(p, e, &nexp);
    err = deal_first_line(p, tn, &ctl->conn);
    if (err) return;
    // 取出请求载荷
    for (p = nexp; p != e; p = nexp) {
        tn = get_line(p, e, &nexp);
        if (tn == 0) {
            p = nexp;
            break;
        }
    }
    ctl->conn.req_cont = p != e ? p : NULL;
    // 解码
    url_decode(ctl->conn.req_url);
    // 检查 url
    check_url(ctl);
    return;
}