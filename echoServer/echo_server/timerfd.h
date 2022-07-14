#ifndef TIMERFD_H_ 
#define TIMERFD_H_


#include "ev2.h"

typedef struct timer_s timer_tt;

timer_tt* timer_new(ev2_loop_t* loop);
void timer_free(timer_tt* t);
int TimerStart(timer_tt* t,int time,void* arg,ev2_poll_cb cb);
//int TimerStop(timer_tt* t);

#endif
