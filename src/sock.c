#include "ds.h"
#include "opt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

int srv_s = 0;
struct sockaddr_in srv_addr;

int init_socket() {
    int err, reuse = 1;
    printf(PR_BLUE "Creating socket...\n" PR_END);
    srv_s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (srv_s < 0) {
        perror("Create socket");
        return 1;
    }
    // 设置端口复用了是为了方便服务器重启
    printf(PR_BLUE "Setting address reusable...\n" PR_END);
    err = setsockopt(srv_s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (err) {
        perror("Setsockopt REUSEADDR");
        return 2;
    }
    // 绑定端口
    printf(PR_BLUE "Binding address...\n" PR_END);
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(final_conf.ListenPort);
    err = bind(srv_s, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (err) {
        perror("Binding address");
        return 3;
    }
    // 监听套接字
    printf(PR_BLUE "Listening...\n" PR_END);
    err = listen(srv_s, final_conf.MaxClient << 1);
    if (err) {
        perror("Listen");
        return 4;
    }
    return 0;
}