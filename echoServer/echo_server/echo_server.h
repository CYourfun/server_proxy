#ifndef ECHO_SERVER_H_
#define ECHO_SERVER_H_

#include "ev2.h"

typedef struct echo_server_s echo_server_t;

typedef struct conn_s conn_t;

echo_server_t *echo_server_new(ev2_loop_t *loop);
void echo_server_free(echo_server_t *s);

int echo_server_listen(echo_server_t *s, int port);

#endif
