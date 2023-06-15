#include "ds.h"
#include "opt.h"
#include "sig.h"
#include "sock.h"
#include "http.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>

pthread_mutex_t thread_init = PTHREAD_MUTEX_INITIALIZER;

worker_ctl *workers;

void do_work(worker_ctl *ctl) {
    printf("Thread (i=%d): Do working.\n", (int)(ctl - workers));
    int err, cs = ctl->conn.cli_s;
    struct pollfd cli_fd = {
        .fd = cs, .events = POLLIN, 
        .revents = 0
    };
    conn_info *conn = &ctl->conn;
    while (!ctl->stop) {
        err = poll(&cli_fd, 1, final_conf.TimeOut * 1000);
        if (err == 0) break; // timeout
        if (err == -1) {
            perror("poll");
            continue;
        }
        printf("Thread (i=%d): Receving.\n", (int)(ctl - workers));
        memset(conn->req_buf, 0, sizeof(conn->req_buf));
        conn->req_len = read(cs, conn->req_buf, sizeof(conn->req_buf));
        if (conn->req_len > 0) {
            ctl->conn.req_fd = -1;
            parse_request(ctl);
            handle_request(ctl);
            if (ctl->conn.req_fd != -1) close(ctl->conn.req_fd);
            usleep(10000);
        } else break;
    }
}

void *worker_main(void *arg) {
    int err;
    worker_ctl *ctl = (worker_ctl *)arg;
    pthread_mutex_unlock(&thread_init); // 通知 init_workers 继续进行
    while (!ctl->stop) {
        err = pthread_mutex_trylock(&ctl->mutex);
        if (err) usleep(10000);
        else {
            ctl->status = STATUS_BUSY;
            do_work(ctl);
            printf("Thread (i=%d): Disconnect.\n", (int)(ctl - workers));
            close(ctl->conn.cli_s);
            ctl->conn.cli_s = -1;
            ctl->status = STATUS_FREE;
        }
    }
    printf("Thread (i=%d) exiting...\n", (int)(ctl - workers));
    ctl->status = STATUS_NONE;
    pthread_exit(NULL);
}

int init_workers() {
    int i, n, err;
    pthread_t tid;
    n = final_conf.MaxClient;
    workers = (worker_ctl *)calloc(sizeof(worker_ctl), n);
    for (i = 0; i < n; i++) {
        // lock 应该在创建前以保证初始状态为 free
        pthread_mutex_init(&workers[i].mutex, NULL);
        pthread_mutex_lock(&workers[i].mutex);
        // 同时只能有一个线程在创建
        pthread_mutex_lock(&thread_init);
        workers[i].status = STATUS_FREE;
        err = pthread_create(&tid, NULL, worker_main, (void *)(workers + i));
        if (err) {
            perror("pthread create");
            return 1;
        }
        // 等待线程创建结束且开始运行
        pthread_mutex_lock(&thread_init);
        pthread_mutex_unlock(&thread_init);
        workers[i].tid = tid;
    }
    return 0;
}

void destory_workers() {
    int i, n;
    n = final_conf.MaxClient;
    printf(PR_BLUE "Waiting for thread stopping...\n" PR_END);
    for (i = 0; i < n; i++) workers[i].stop = 1;
    usleep(100000);
    free(workers);
}

int find_free_worker() {
    int i, n;
    n = final_conf.MaxClient;
    for (i = 0; i < n; i++) {
        if (workers[i].status == STATUS_FREE && workers[i].stop == 0)
            return i;
    }
    return -1;
}

void main_loop() {
    int err, cs;
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    struct pollfd srv_fd = {
        .fd = srv_s, .events = POLLIN, 
        .revents = 0
    };
    while (!stop_all) {
        int free_i = find_free_worker();
        if (free_i == -1) continue;
        err = poll(&srv_fd, 1, 50); // 50ms
        if (err == 0) continue; // 超时
        if (err == -1) {
            perror("poll failed"); // 出错
            continue;
        }
        cs = accept(srv_s, (struct sockaddr *)&cli_addr, &len);
        if (cs < 0) {
            perror("Accept");
            continue;
        }
        printf("Thread (i=%d): Connect with %s\n", free_i, inet_ntoa(cli_addr.sin_addr));
        workers[free_i].status = STATUS_BUSY;
        workers[free_i].conn.cli_s = cs;
        pthread_mutex_unlock(&workers[free_i].mutex);
        usleep(50000);
    }
    destory_workers();
    close(srv_s);
}
