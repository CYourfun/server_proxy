#ifndef EV2_H_
#define EV2_H_

typedef struct ev2_loop_s ev2_loop_t;

ev2_loop_t *ev2_loop_new();
void ev2_loop_free(ev2_loop_t *loop);
int ev2_loop_run(ev2_loop_t *loop);
int ev2_loop_break(ev2_loop_t *loop);


typedef struct ev2_poll_s ev2_poll_t;
typedef void(*ev2_poll_cb)(void *arg, int fd, int what);

enum
{
    EV2_READABLE = 1,
    EV2_WRITABLE = 2,
    EV2_DISCONNECT = 4,
    EV2_PRIORITIZED = 8,
};

ev2_poll_t *ev2_poll_new(ev2_loop_t *loop);
void ev2_poll_free(ev2_poll_t *io);
int ev2_poll_register(ev2_poll_t *io, int fd, int what, void *arg, ev2_poll_cb cb);


#endif
