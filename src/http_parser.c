#include "ds.h"
#include "opt_common.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

const char methods[][10] = {
    "GET", "POST", "UNKNOWN"
};

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
    conn->method = 0;
    for (i = 0; i < SUPPORT_METHODS; i++) {
        if (strcmp(s, methods[i]) == 0) {
            conn->method = i;
            break;
        }
    }
    if (!conn->method) {
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

void check_url(worker_ctl *ctl) {
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
    len1 = strlen(final_conf.DocumentRoot);
    len2 = strlen(ctl->conn.req_url);
    if (len1 + len2 >= mxlen - 125) {
        printf("Too large url\n");
        ctl->conn.req_err = HTTP_URL_TOO_LARGE;
        return;
    }
    strcpy(buf, final_conf.DocumentRoot);
    strcpy(buf + len1, ctl->conn.req_url);
    buf[len1 + len2] = 0;
    // 确认目录/文件是否存在
    if (access(buf, R_OK) != 0) {
        ctl->conn.req_err = HTTP_NOT_FOUND;
        return;
    }
}

void parse_request(worker_ctl *ctl) {
    int err, tn, n; 
    char *p, *e, *nexp;
    n = ctl->conn.req_len;
    p = ctl->conn.req_buf;
    e = p + n;
    // 处理第一行: Method url HTTP/x.x
    tn = get_line(p, e, &nexp);
    err = deal_first_line(p, tn, &ctl->conn);
    // 取出请求载荷
    for (p = nexp; p != e; p = nexp) {
        tn = get_line(p, e, &nexp);
        if (tn == 0) {
            p = nexp;
            break;
        }
    }
    // 检查 url
    check_url(ctl);
    ctl->conn.req_cont = p != e ? p : NULL;
    return;
}