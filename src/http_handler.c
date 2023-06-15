#include "ds.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
}

const char s400[] = "<html><head>Error 400</head><body>Failed to parse this request!</body></html>";
const char s404[] = "<html><head>Error 404</head><body>File not found!</body></html>";
const char s414[] = "<html><head>Error 414</head><body>Too large url!</body></html>";
const char s503[] = "<html><head>Error 503</head><body>Service Failed!</body></html>";

void do_error(worker_ctl *ctl) {
    const char *s;
    conn_info *conn = &ctl->conn;
    printf("Request Error: %d\n", conn->req_err);
    switch (conn->req_err) {
        case 400: s = s400; break;
        case 404: s = s404; break;
        case 414: s = s414; break;
        case 503: s = s503; break;
        default: s = s400; break;
    }
    gen_response_hdr(ctl, sizeof(s));
    write(conn->cli_s, conn->rsp_buf, conn->rsp_len);
    write(conn->cli_s, s, sizeof(s));
}

void do_method(worker_ctl *ctl) {
    
}

void handle_request(worker_ctl *ctl) {
    printf("Method: %s, url: %s\n", methods[ctl->conn.method], ctl->conn.req_url);
    if (ctl->conn.req_cont) printf("Content: %s\n", ctl->conn.req_cont);
    switch (ctl->conn.req_err) {
        case HTTP_OK: do_method(ctl); break;
        default: do_error(ctl); break;
    }
}