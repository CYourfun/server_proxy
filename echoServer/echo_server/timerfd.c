#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <sys/socket.h>

#include "timerfd.h"
#include "ev2.h"
#include "echo_server.h"

struct timer_s//combine with epoll
{
    ev2_loop_t* loop;
    ev2_poll_t* poll;
    ev2_poll_cb cb;
    int fd1;
    //int SwitchOff; 
};

timer_tt* timer_new(ev2_loop_t* loop)
{
    timer_tt* t = (timer_tt*)malloc(sizeof(timer_tt));
    if (t != NULL) {
        memset(t, 0, sizeof(timer_tt));
        t->loop = loop;
        t->poll = ev2_poll_new(loop);
        if (t->poll == NULL) {
            free(t);
            return NULL;
        }
        t->fd1 = -1;
    }
    return t;
}

void timer_free(timer_tt* t)
{
    if (t != NULL) {
        ev2_poll_free(t->poll);

        if (t->fd1 != -1) {
            close(t->fd1);
            t->fd1 = -1;
        }
        free(t);
    }
}

//单位：毫秒
int TimerStart(timer_tt* t, int time, void* arg, ev2_poll_cb cb)
{
    //if (t->SwitchOff == 1)
    //{
    //    return 0;
    //}
    int err;
    int flag = 0;
    struct itimerspec new_value;
    /*init time*/
    new_value.it_value.tv_sec = time/1000;
    new_value.it_value.tv_nsec = (time%1000)*1000000;
    /*time interval*/
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;

    if(t->fd1 == -1)
    {
        t->fd1 = timerfd_create(CLOCK_MONOTONIC, 0);
        printf("------1TIMER ON1------\n");
        if (t->fd1 < 0) {
            perror("timerfd create error");
            return -1;
        }
        flag = 1;
    }
    else
    {
        printf("timer update\n");
    }
 
    int ret = timerfd_settime(t->fd1, 0, &new_value, NULL);//启动定时器
    if (ret < 0) {
        perror("timer_settime error");
        close(t->fd1);
        return -1;
    }

    if (flag == 1)
    {
        err = ev2_poll_register(t->poll,
            t->fd1,
            EV2_READABLE,
            arg,
            cb);
        if (err < 0)
        {
            close(t->fd1);
            t->fd1 = -1;
            return -1;
        }
    }

    return 0;
}

//int TimerStop(timer_tt* t)
//{
//    struct itimerspec new_value;
//
//    new_value.it_value.tv_sec = 0;
//    new_value.it_value.tv_nsec = 0;
//
//    new_value.it_interval.tv_sec = 0;
//    new_value.it_interval.tv_nsec = 0;
//
//    int ret = timerfd_settime(t->fd1, 0, &new_value, NULL);
//    if (ret < 0) {
//        perror("timer_settime error");
//        close(t->fd1);
//        return -1;
//    }
//
//    t->SwitchOff = 1;
//     
//    int err = ev2_poll_register(t->poll,
//        -1,
//        0,
//        NULL,
//        NULL);
//
//    printf("------TIMER STOP------\n");
//}