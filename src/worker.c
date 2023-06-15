#include "ds.h"
#include "opt_common.h"
#include "sig.h"
#include "sock.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

pthread_mutex_t thread_init = PTHREAD_MUTEX_INITIALIZER;

worker_ctl *workers;

void do_work(worker_ctl *ctl) {
    
}

void *worker_main(void *arg) {
    int err;
    worker_ctl *ctl = (worker_ctl *)arg;
    pthread_mutex_unlock(&thread_init); // 通知 init_workers 继续进行
    while (!ctl->stop) {
        err = pthread_mutex_trylock(&ctl->mutex);
        if (err) sleep(1);
        else {
            ctl->status = STATUS_BUSY;
            do_work(ctl);
            ctl->status = STATUS_FREE;
            close(ctl->conn.cli_s);
            ctl->conn.cli_s = -1;
        }
    }
    printf("Thread (tid=%d) exiting...\n", (int)ctl->tid);
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
    sleep(2);
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
        err = poll(&srv_fd, 1, 500); // 500ms
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
        workers[free_i].status = STATUS_BUSY;
        workers[free_i].conn.cli_s = cs;
        pthread_mutex_unlock(&workers[free_i].mutex);
    }
    destory_workers();
    close(srv_s);
}
