#include "ev2.h"
#include "echo_server.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/timerfd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if 0
#define INFO(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define INFO(fmt, ...)
#endif

typedef struct conn_s conn_t;

struct conn_s
{
    ev2_loop_t *loop;
    ev2_poll_t *poll;
	ev2_poll_t *poll2;
    ev2_poll_t* poll3;
    int fd1;
	int fd2;
    int fd3;//timefd
    timer_tt *t;//++ ++ ++
};

conn_t *conn_new(ev2_loop_t *loop, int fd)
{
    conn_t *conn = (conn_t *)malloc(sizeof(conn_t));
    if (conn != NULL) {
        memset(conn, 0, sizeof(conn_t));
        conn->loop = loop;
        conn->poll = ev2_poll_new(loop);
		if (conn->poll == NULL) {
			free(conn);
			return NULL;
		}
		conn->poll2 = ev2_poll_new(loop);
		if (conn->poll2 == NULL) {
			ev2_poll_free(conn->poll);
			free(conn);
			return NULL;
		}
        conn->poll3 = ev2_poll_new(loop);
        if (conn->poll3 == NULL) {
            ev2_poll_free(conn->poll);
            ev2_poll_free(conn->poll2);
            free(conn);
            return NULL;
        }
        conn->fd1 = fd;
    }
    return conn;
}

void conn_free(conn_t *conn)
{
    if (conn != NULL) {
        ev2_poll_free(conn->poll);
		ev2_poll_free(conn->poll2);
        //ev2_poll_free(conn->poll3);
        if (conn->fd1 != -1) {
            close(conn->fd1);
            conn->fd1 = -1;
        }
		if (conn->fd2 != -1)//++ ++
		{
			close(conn->fd2);
			conn->fd2 = -1;
		}
        if (conn->fd3 != -1)//++ ++
        {
            close(conn->fd3);
            conn->fd3 = -1;
        }
        free(conn);
    }
}

struct echo_server_s
{
    ev2_loop_t *loop;
    ev2_poll_t *poll;
    int fd;

};

echo_server_t *echo_server_new(ev2_loop_t *loop)
{
    echo_server_t *s = (echo_server_t *)malloc(sizeof(echo_server_t));
    if (s != NULL) {
        memset(s, 0, sizeof(echo_server_t));
        s->loop = loop;
        s->poll = ev2_poll_new(loop);
        if (s->poll == NULL) {
            free(s);
            return NULL;
        }
        s->fd = -1;
    }
    return s;
}

void echo_server_free(echo_server_t *s)
{
    if (s != NULL) {
        ev2_poll_free(s->poll);
        if (s->fd != -1) {
            close(s->fd);
            s->fd = -1;
        }
        free(s);
    }
}

struct timer_s//++ ++ ++ combine with epoll
{
    ev2_loop_t* loop;
    ev2_poll_t* poll;
    int fd1;

};

timer_tt *timer_new(ev2_loop_t* loop) //++ ++ ++
{
    timer_tt *t = (timer_tt *)malloc(sizeof(timer_tt));
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

static int simple_send(int fd, const char *buf, size_t len)
{
    size_t sent = 0;
    ssize_t r;

    while (sent < len) {
        r = send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            perror("send");
            return -1;
        }
        sent += r;
    }
    return 0;
}

static void timer_out(void* arg, int fd, int what)//++ ++ ++
{
    printf("--------TIME OUT--------\n");

    uint64_t exp;
    ssize_t size;
    conn_t* c = (conn_t*)arg;
    int err;

    if (what & EV2_READABLE)
    {
        size = read(fd, &exp, sizeof(uint64_t)); //����uint64_t��С
        if (size == sizeof(uint64_t))
        {
            printf("2---TIME OUT DISCONNECT---2\n");
            err = shutdown(c->fd1,SHUT_WR);
            if (err < 0)
            {
                perror("shutdown error");
            }
            err = shutdown(c->fd2, SHUT_WR);
            if (err < 0)
            {
                perror("shutdown error");
            }
            conn_free(c);
        }
    }
    close(fd);
    timer_free(c->t);
}

static void timer_update(int t)//++ ++ ++
{
    int err;
    printf("------BEGIN TIMER UPDATE------\n");
    //conn_t* t = (conn_t*)arg;
    struct itimerspec its;

    its.it_value.tv_sec = 10;
    its.it_value.tv_nsec = 0;

    its.it_interval.tv_sec = 10;
    its.it_interval.tv_nsec = 0;

    if (timerfd_settime(t, 0, &its, NULL) < 0)
    {
        close(t);
        return;
    }
    printf("timer update\n");

    return;
}

static void echo_server__on_readable(void *arg, int fd, int what)
{
    conn_t *conn = (conn_t *)arg;
    char buf[4096];
    ssize_t len;
    int err;
    int count = 0;

    if (what < 0) {
        conn_free(conn);
        return;
    }
    
    timer_update(conn->fd3);//Synchronized trigger

    if (what & EV2_DISCONNECT) {
        INFO("disconnect fd=%d\n", conn->fd1);
        shutdown(conn->fd1, SHUT_WR);
        conn_free(conn);
        return;
    }

    if (what & EV2_READABLE) {
        while (1) {
            len = recv(conn->fd1, buf, sizeof(buf), MSG_DONTWAIT);
            if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

                perror("recv");
                conn_free(conn);
                return;
            }
            else if (len == 0) {
                INFO("disconnect fd=%d\n", conn->fd1);
                shutdown(conn->fd1, SHUT_WR);

                conn_free(conn);
                return;
            }
            else {
                err = simple_send(conn->fd2, buf, (size_t)len);
                if (err < 0) {            
                    conn_free(conn);
                    return;
                }
                if (len < (int)ARRAY_SIZE(buf) ||
                    count++ >= 1) {
                    break;
                }
            }
        }
    }
}

static void echo_server__on_readable2(void *arg, int fd, int what)
{
	conn_t *conn = (conn_t *)arg;
	char buf[4096];
	ssize_t len;
	int err;
	int count = 0;

	if (what < 0) {
		conn_free(conn);
		return;
	}

    timer_update(conn->fd3);//Synchronized trigger

	if (what & EV2_DISCONNECT) {
		INFO("disconnect fd=%d\n", conn->fd);
		shutdown(conn->fd2, SHUT_WR);//++ ++
		conn_free(conn);
		return;
	}

	if (what & EV2_READABLE) {
		while (1) {
			len = recv(conn->fd2, buf, sizeof(buf), MSG_DONTWAIT);
			if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

				perror("recv");
				conn_free(conn);
				return;
			}
			else if (len == 0) {
				INFO("disconnect fd=%d\n", conn->fd2);
				shutdown(conn->fd2, SHUT_WR);//++ ++
				conn_free(conn);
				return;
			}
			else {
				err = simple_send(conn->fd1, buf, (size_t)len);
				if (err < 0) {               
					conn_free(conn);
					return;
				}
				if (len < (int)ARRAY_SIZE(buf) ||
					count++ >= 1) {
					break;
				}
			}
		}
	}
}

static void echo_server__on_new_connection(void *arg, int fd, int what)
{
    echo_server_t *s = (echo_server_t *)arg;
    struct sockaddr_in6 sa;
	
	struct sockaddr_in6 sa2;//++
	memset(&sa2, 0, sizeof(sa2));
	sa2.sin6_family = AF_INET6;
	sa2.sin6_addr.__in6_u.__u6_addr32[4] = inet_addr("192.168.91.128");
	sa2.sin6_port = htons(8080);

    socklen_t sa_len;//++ ++
    int cfd;
    int err;

    if (what < 0) {
        echo_server_free(s);
        return;
    }

    if (what & EV2_READABLE) {
        memset(&sa, 0, sizeof(struct sockaddr_in6));
        sa_len = sizeof(struct sockaddr_in6);
        
        cfd = accept(s->fd, &sa, &sa_len);//accept		
        if (cfd < 0) {
            perror("accept");
            if (errno == EMFILE ||
                errno == EAGAIN) {
                return;
            }
            echo_server_free(s);
            return;
        }
      
        conn_t *conn = conn_new(s->loop, cfd);
		if (conn == NULL) {
			close(cfd);
			return;
		}
		      
        timer_tt* t = timer_new(conn->loop);//++ ++ ++
       
	    conn->fd2 = socket(AF_INET6, SOCK_STREAM, 0);//++
		if (conn->fd2 < 0)
		{
			perror("socket error");
			abort();
		} 
		int con = connect(conn->fd2, (struct sockaddr*)&sa2, sizeof(sa2));//++
		if (con < 0)
	    {
			perror("connection error");
            conn_free(conn);
            return;
		}

        conn->t = t;//�����ͷ�t
        int timefd = TimerFdInit(t, conn);//++ ++ ++
        if (timefd < 0)
        {
            printf("TimerFdInit fail\n");
            timer_free(t);
        }
        conn->poll3 = t->poll;
        conn->fd3 = t->fd1;

        err = ev2_poll_register(conn->poll,cfd, EV2_READABLE, conn, echo_server__on_readable);
        if (err < 0) {
            //perror("epoll_ctl");
			printf("register false\n");
            conn_free(conn);
            return;
        }

		err = ev2_poll_register(conn->poll2, conn->fd2, EV2_READABLE, conn, echo_server__on_readable2);//++
		if (err < 0) {
			//perror("epoll_ctl");
			conn_free(conn);
            return;
		}

        INFO("new connection fd=%d\n", conn->fd);
    }
}

int echo_server_listen(echo_server_t *s, int port)
{
    struct sockaddr_in6 sa;
    int err;
    int opt = 1;
    int flags;

    if (s->fd != -1) {
        ev2_poll_register(s->poll, -1, 0, NULL, NULL);
        close(s->fd);
        s->fd = -1;
    }

    s->fd = socket(PF_INET6, SOCK_STREAM, 0);//socket
    if (s->fd < 0) {
        perror("socket");
        abort();
    }

    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&sa, 0, sizeof(struct sockaddr_in6));
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons((uint16_t)port);

    err = bind(s->fd, (struct sockaddr *)&sa, sizeof(sa));//bind
    if (err < 0) {
        close(s->fd);
        s->fd = -1;
        perror("bind");
        return -1;
    }

    err = listen(s->fd, 100);//listen
    if (err < 0) {
        close(s->fd);
        s->fd= -1;
        perror("listen");
        return -1;
    }

    // nonblocking accept.
    flags = fcntl(s->fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(s->fd, F_SETFL, flags);

    err = ev2_poll_register(s->poll,
                            s->fd,
                            EV2_READABLE,
                            s,
                            echo_server__on_new_connection);

    if (err < 0) {
        close(s->fd);
        s->fd = -1;
        //perror("epoll_ctl");
        return -1;
    }

    return 0;
}

int TimerFdInit(timer_tt *t,conn_t *c)
{
    int err;
     
    struct itimerspec new_value;
    /*init time*/
    new_value.it_value.tv_sec = 10;
    new_value.it_value.tv_nsec = 0;
    /*time interval*/
    new_value.it_interval.tv_sec = 10;
    new_value.it_interval.tv_nsec = 0;

    t->fd1= timerfd_create(CLOCK_MONOTONIC, 0);
    if (t->fd1 < 0) {
        perror("timerfd create error");
        return -1;
    }

    int ret = timerfd_settime(t->fd1, 0, &new_value, NULL);//������ʱ��
    if (ret < 0) {
        perror("timer_settime error");
        close(t->fd1);
        return -1;
    }
    printf("------1TIMER ON1------\n");

    err= ev2_poll_register(t->poll,      
        t->fd1,
        EV2_READABLE,
        c,
        timer_out);
    if (err < 0)
    {
        close(t->fd1);
        t->fd1 = -1;
        return -1;
    }
    return 0;
}



