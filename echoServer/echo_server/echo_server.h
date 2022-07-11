#ifndef ECHO_SERVER_H_
#define ECHO_SERVER_H_

#include "ev2.h"

typedef struct echo_server_s echo_server_t;
typedef struct timer_s timer_tt;
typedef struct conn_s conn_t;

echo_server_t *echo_server_new(ev2_loop_t *loop);
void echo_server_free(echo_server_t *s);

timer_tt *timer_new(ev2_loop_t *loop);
void timer_free(timer_tt *t);
int TimerFdInit(timer_tt* t,conn_t* c);

int echo_server_listen(echo_server_t *s, int port);


#endif
