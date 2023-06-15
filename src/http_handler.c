#include "ds.h"
#include "http.h"
#include "worker.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

const char TEST_CONTENT[] = "<html><head>Hello World!</head><body><h3>This is the body!</h3></body></html>";
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

static inline int min(int a, int b) {
    return a > b ? b : a;
}

void gen_response_hdr(worker_ctl *ctl, int len) {
    int n;
    conn_info *conn = &ctl->conn;
    char *p = conn->rsp_buf;
    memset(p, 0, sizeof(conn->rsp_buf));
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
    int n, err;
    conn_info *conn = &ctl->conn;
    gen_response_hdr(ctl, conn->fd_stat.st_size);
    err = write(conn->cli_s, conn->rsp_buf, conn->rsp_len);
    if (err < 0) perror("write in do_get");
    for (;;) {
        n = read(conn->req_fd, conn->rsp_buf, sizeof(conn->rsp_buf));
        if (n > 0) {
            err = write(conn->cli_s, conn->rsp_buf, n);
            if (err < 0) perror("write in do_get");
        } else if (n < 0) {
            perror("Read");
            break;
        } else break;
    }
}

void do_post(worker_ctl *ctl) {
    int n, err;
    conn_info *conn = &ctl->conn;
    if (access(conn->rsp_buf, X_OK) != 0) {
        goto bad;
    }
    if (conn->req_cont == NULL) goto bad;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        goto bad;
    }
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        goto bad;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        char *p = conn->req_cont;
        int i = 1, cnt = 1;
        while (*p != 0) {
            if (*p == '&') *p = 0, ++cnt;
            p++;
        }
        char **argv = (char **)calloc(sizeof(char *), cnt + 2);
        argv[0] = conn->rsp_buf;
        p = conn->req_cont;
        argv[1] = p;
        while (i < cnt) {
            if (*p == 0) argv[++i] = p + 1;
            p++;
        }
        argv[cnt + 1] = NULL;
        for (i = 1; i <= cnt; i++) url_decode(argv[i]);
        for (i = 0; i <= cnt; i++) {
            fprintf(stderr, "%s ", argv[i]);
        }
        fprintf(stderr, "\n");
        execv(argv[0], argv);
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        close(conn->req_fd);
        conn->req_fd = pipefd[0];
        wait(NULL);
        if (fstat(conn->req_fd, &conn->fd_stat) != 0) {
            perror("fstat");
        }
        if (conn->fd_stat.st_size == 0) conn->fd_stat.st_size = 100;
        do_get(ctl);
    }
    return;
bad:
    conn->req_err = 503;
    do_error(ctl);
    return;
}

void do_method(worker_ctl *ctl) {
    int n, err;
    conn_info *conn = &ctl->conn;
    switch (conn->method) {
        case HTTP_GET: do_get(ctl); break;
        case HTTP_POST: do_post(ctl); break;
        default: break;
    }
}

void handle_request(worker_ctl *ctl) {
    printf("Thread (i=%d): Method: %s, url: %s\n", 
        (int)(ctl - workers), methods[ctl->conn.method], ctl->conn.req_url);
    if (ctl->conn.req_cont) printf("Content: %s\n", ctl->conn.req_cont);
    switch (ctl->conn.req_err) {
        case HTTP_OK: do_method(ctl); break;
        default: do_error(ctl); break;
    }
}