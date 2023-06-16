#include "ds.h"
#include "http.h"
#include "worker.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

const char TEST_CONTENT[] = "<html><head>Hello World!</head><body><h3>This is the body!</h3></body></html>";

static inline int min(int a, int b) {
    return a > b ? b : a;
}

void gen_response_hdr(worker_ctl *ctl, int len) {
    int n;
    conn_info *conn = &ctl->conn;
    char *p = conn->rsp_buf;
    memset(p, 0, sizeof(conn->rsp_buf));
    conn->rsp_len = 0;
    n = sprintf(p, "HTTP/1.1 %d %s\r\n", conn->req_err, err2str(conn->req_err));
    p += n, conn->rsp_len += n;
    n = sprintf(p, "Content-Type: text/html\r\n");
    p += n, conn->rsp_len += n;
    n = sprintf(p, "Content-Length: %d\r\n\r\n", len);
    p += n, conn->rsp_len += n;
    printf("Thread (i=%d): response header:\n%s", (int)(ctl - workers), conn->rsp_buf);
}

const char s400[] = "<html><head><title>Error 400</title></head>\n<body>Failed to parse this request!</body></html>";
const char s404[] = "<html><head><title>Error 404</title></head>\n<body>File not found!</body></html>";
const char s414[] = "<html><head><title>Error 414</title></head>\n<body>Too large url!</body></html>";
const char s503[] = "<html><head><title>Error 503</title></head>\n<body>Service Failed!</body></html>";

void do_error(worker_ctl *ctl) {
    int err, len;
    const char *s;
    conn_info *conn = &ctl->conn;
    printf("Thread (i=%d): Request Error: %d\n", (int)(ctl - workers), conn->req_err);
    switch (conn->req_err) {
        case 400: s = s400; break;
        case 404: s = s404; break;
        case 414: s = s414; break;
        case 503: s = s503; break;
        default: s = s400; break;
    }
    len = strlen(s);
    gen_response_hdr(ctl, len);
    err = write(conn->cli_s, conn->rsp_buf, conn->rsp_len);
    if (err < 0) perror("write in do_error1");
    err = write(conn->cli_s, s, len);
    if (err < 0) perror("write in do_error2");
}

void do_get(worker_ctl *ctl) {
    int n, err, p;
    conn_info *conn = &ctl->conn;
    gen_response_hdr(ctl, conn->fd_stat.st_size);
    err = write(conn->cli_s, conn->rsp_buf, conn->rsp_len);
    printf("rsp_len = %d\n", conn->rsp_len);
    if (err <= 0) goto bad;
    for (;;) {
        n = read(conn->req_fd, conn->rsp_buf, sizeof(conn->rsp_buf));
        if (n == 0) break;
        if (n > 0) {
            p = 0;
            while(n > 0) {
                err = write(conn->cli_s, conn->rsp_buf + p, n);
                if (err <= 0) goto bad;
                p = err; n -= err;
            }
        } else if (n < 0) goto bad;
    }
    return;
bad:
    perror("do_get");
    return;
}

void do_post(worker_ctl *ctl) {
    int n, err;
    conn_info *conn = &ctl->conn;
    // 目标文件必须可执行
    if (access(conn->rsp_buf, X_OK) != 0) {
        goto bad;
    }
    if (conn->cont_len == 0 || conn->req_cont == NULL) goto bad;
    if (conn->cont_len > 1000000) goto bad;
    // 将 content 提取完整
    char *new_buf = (char *)calloc(1, conn->cont_len + 3);
    if (new_buf == NULL) {
        perror("calloc");
        goto bad;
    }
    n = conn->req_buf + conn->req_len - conn->req_cont;
    memcpy(new_buf, conn->req_cont, n);
    if (n < conn->cont_len) {
        err = read(conn->cli_s, new_buf + n, conn->cont_len - n);
        if (err < 0) {
            perror("read in do_post");
            goto bad2;
        }
    }
    // 生成 argv
    char **argv = form2argv(conn->rsp_buf, new_buf, NULL);
    // 输出转发到管道
    int pipefd = exec2pipe(argv);
    // 原本打开的这个描述符不需要了
    close(conn->req_fd); 
    // 管道转存到临时文件
    conn->req_fd = stream2file((int)(ctl - workers), pipefd);
    // 释放不需要的资源
    close(pipefd);
    free(argv);
    if (conn->req_fd == -1) goto bad3;
    if (fstat(conn->req_fd, &conn->fd_stat) != 0) goto bad3;
    // 响应文件资源
    do_get(ctl);
    return;
bad3:
    free(argv);
bad2:
    free(new_buf);
bad:
    conn->req_err = 503;
    do_error(ctl);
    return;
}

void do_method(worker_ctl *ctl) {
    if (ctl->stop) return;
    int n, err;
    conn_info *conn = &ctl->conn;
    switch (conn->method) {
        case HTTP_GET: do_get(ctl); break;
        case HTTP_POST: do_post(ctl); break;
        default: break;
    }
}

void handle_request(worker_ctl *ctl) {
    if (ctl->stop) return;
    printf("Thread (i=%d): Method: %s, url: %s\n", 
        (int)(ctl - workers), methods[ctl->conn.method], ctl->conn.req_url);
    if (ctl->conn.req_cont) 
        printf("Len: %d, Content: %s\n", ctl->conn.cont_len, ctl->conn.req_cont);
    switch (ctl->conn.req_err) {
        case HTTP_OK: do_method(ctl); break;
        default: do_error(ctl); break;
    }
}