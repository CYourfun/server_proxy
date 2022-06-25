
#include "ev2.h"
#include "queue.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


/* ASSERT() is for debug checks, CHECK() for run-time sanity checks.
 * DEBUG_CHECKS is for expensive debug checks that we only want to
 * enable in debug builds but still want type-checked by the compiler
 * in release builds.
 */
#if defined(NDEBUG)
# define ASSERT(exp)
# define CHECK(exp)    do { if (!(exp)) abort(); } while (0)
# define DEBUG_CHECKS  (0)
#else
# define ASSERT(exp)   assert(exp)
# define CHECK(exp)    assert(exp)
# define DEBUG_CHECKS  (1)
#endif


struct ev2_poll_s
{
    QUEUE queue;
    ev2_loop_t *loop;
    int fd;
    int events;
    void *arg;
    ev2_poll_cb poll_cb;
};



struct ev2_loop_s
{
    int epfd;
    int stop;
    QUEUE poll_handles;

    unsigned int nfds;
    unsigned int npolls;
    ev2_poll_t **polls;
};

ev2_loop_t *ev2_loop_new()
{
    ev2_loop_t *loop = (ev2_loop_t *)malloc(sizeof(ev2_loop_t));
    if (loop != NULL) {
        memset(loop, 0, sizeof(ev2_loop_t));

        loop->epfd = epoll_create1(O_CLOEXEC);
        if (loop->epfd == -1 && (errno == ENOSYS || errno == EINVAL)) {
            loop->epfd = epoll_create(256);

            if (loop->epfd != -1) {
                fcntl(loop->epfd, F_SETFD, FD_CLOEXEC);
            }
        }
        if (loop->epfd < 0) {
            perror("epoll_create");
            abort();
        }
        QUEUE_INIT(&loop->poll_handles);
    }
    return loop;
}

void ev2_loop_free(ev2_loop_t *loop)
{
    QUEUE *node;
    ev2_poll_t *poll;

    if (loop != NULL) {
        while (!QUEUE_EMPTY(&loop->poll_handles)) {
            node = QUEUE_HEAD(&loop->poll_handles);
            poll = QUEUE_DATA(node, ev2_poll_t, queue);
            QUEUE_REMOVE(node);

            poll->loop = NULL;
            if (poll->events != 0) {
                epoll_ctl(loop->epfd, EPOLL_CTL_DEL, poll->fd, NULL);
                poll->events = 0;
                loop->nfds -= 1;
                loop->polls[poll->fd] = NULL;

                poll->poll_cb(poll->arg, poll->fd, -1);
            }
        }

        ASSERT(loop->nfds == 0);

        close(loop->epfd);
        free(loop->polls);
        free(loop);
    }
}

int ev2_loop_run(ev2_loop_t *loop)
{
    struct epoll_event events[1024];
    struct epoll_event *pe;
    int i;
    int nfds;
    int what;
    ev2_poll_t *poll;

    while (1) {
        if (loop->nfds == 0)
            break;

        nfds = epoll_wait(loop->epfd, events, 1024, -1);
        if (nfds < 0) {
            if (errno != EINTR) {
                perror("epoll_wait");
                abort();
            }
            // TODO: should be continue?
            return -1;
        }

        for (i = 0; i < nfds; ++i) {
            pe = &events[i];
            ASSERT((unsigned int)pe->data.fd < loop->npolls);
            ASSERT(loop->polls[pe->data.fd] != NULL);

            poll = loop->polls[pe->data.fd];
            pe->events &= poll->events | EPOLLERR | EPOLLHUP;

            if (pe->events == EPOLLERR || pe->events == EPOLLHUP) {
                pe->events |= poll->events & (EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI);
            }
            if (pe->events != 0) {
                what = 0;
                if (pe->events & EPOLLIN)
                    what |= EV2_READABLE;
                if (pe->events & EPOLLPRI)
                    what |= EV2_PRIORITIZED;
                if (pe->events & EPOLLOUT)
                    what |= EV2_WRITABLE;
                if (pe->events & EPOLLRDHUP)
                    what |= EV2_DISCONNECT;
                poll->poll_cb(poll->arg, poll->fd, what);
            }
        }
    }

    return 0;
}
int ev2_loop_break(ev2_loop_t *loop)
{
    // TODO:
    return -1;
}


ev2_poll_t *ev2_poll_new(ev2_loop_t *loop)
{
    ev2_poll_t *poll = (ev2_poll_t *)malloc(sizeof(ev2_poll_t));
    if (poll != NULL) {
        memset(poll, 0, sizeof(ev2_poll_t));

        poll->loop = loop;
        QUEUE_INSERT_TAIL(&loop->poll_handles, &poll->queue);
    }
    return poll;
}

void ev2_poll_free(ev2_poll_t *poll)
{
    if (poll != NULL) {
        if (poll->loop != NULL) {
            ev2_poll_register(poll, -1, 0, NULL, NULL);
            poll->loop = NULL;
            QUEUE_REMOVE(&poll->queue);
        }
        free(poll);
    }
}

static unsigned int next_power_of_two(unsigned int val)
{
    val -= 1;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val += 1;
    return val;
}

static void maybe_resize(ev2_loop_t *loop, unsigned int len)
{
    ev2_poll_t **polls;
    unsigned int npolls;
    unsigned int i;

    if (len <= loop->npolls)
        return;

    npolls = next_power_of_two(len);
    polls = (ev2_poll_t **)realloc(loop->polls,
                                   npolls * sizeof(ev2_poll_t *));

    if (polls == NULL) {
        perror("realloc");
        abort();
    }
    for (i = loop->npolls; i < npolls; ++i)
        polls[i] = NULL;

    loop->polls = polls;
    loop->npolls = npolls;
}

int ev2_poll_register(ev2_poll_t *poll, int fd, int what, void *arg, ev2_poll_cb poll_cb)
{
    struct epoll_event ev;
    int events = 0;
    int opcode = EPOLL_CTL_ADD;

    if (poll->loop == NULL)
        return -1;

    if (what & EV2_READABLE)
        events |= EPOLLIN;
    if (what & EV2_PRIORITIZED)
        events |= EPOLLPRI;
    if (what & EV2_WRITABLE)
        events |= EPOLLOUT;
    if (what & EV2_DISCONNECT)
        events |= EPOLLRDHUP;
    //if (events != 0)
    //    events |= EPOLLERR | EPOLLHUP;

    if (fd < 0 || events == 0) {
        if (poll->events != 0) {
            epoll_ctl(poll->loop->epfd, EPOLL_CTL_DEL, poll->fd, NULL);
            poll->events = 0;
            poll->loop->nfds -= 1;
            poll->loop->polls[poll->fd] = NULL;
        }
    }
    if (fd >= 0 && events != 0) {
        ASSERT(poll_cb != NULL);
        if (poll->events != 0) {
            if (poll->fd == fd)
                opcode = EPOLL_CTL_MOD;
            else {
                epoll_ctl(poll->loop->epfd, EPOLL_CTL_DEL, poll->fd, NULL);
                poll->events = 0;
                poll->loop->nfds -= 1;
                poll->loop->polls[poll->fd] = NULL;
            }
        }

        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = events;

        if (epoll_ctl(poll->loop->epfd, opcode, fd, &ev)) {
            if (errno != EEXIST) {
                perror("epoll_ctl");
                abort();
            }
            ASSERT(opcode == EPOLL_CTL_ADD);

            //if (epoll_ctl(poll->loop->epfd, EPOLL_CTL_MOD, fd, &ev)) {
            //    perror("epoll_ctl");
            //    abort();
            //}
            return -1;
        }
        poll->fd = fd;
        poll->events = events;
        poll->arg = arg;
        poll->poll_cb = poll_cb;

        maybe_resize(poll->loop, fd + 1);
        if (opcode == EPOLL_CTL_ADD)
            poll->loop->nfds += 1;
        poll->loop->polls[fd] = poll;
    }

    return 0;
}
