#include "ds.h"
#include "opt_common.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

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
    printf("Waiting for thread stopping...\n");
    for (i = 0; i < n; i++) workers[i].stop = 1;
    sleep(2);
    free(workers);
}