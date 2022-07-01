
#include "ev2.h"
#include "echo_server.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if 1
#define INFO(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define INFO(fmt, ...)
#endif

typedef struct conn_s conn_t;

struct conn_s
{
    ev2_loop_t *loop;
    ev2_poll_t *poll;
    int fd;
	int fd2;//用于connect
	unsigned int bytes;
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
        conn->fd = fd;
    }
    return conn;
}

conn_t *conn_new2(ev2_loop_t *loop, int fd)//++
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
        conn->fd2 = fd;
    }
    return conn;
}

void conn_free(conn_t *conn)
{
    if (conn != NULL) {
        ev2_poll_free(conn->poll);
        if (conn->fd != -1) {
            close(conn->fd);
            conn->fd = -1;
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

    if (what & EV2_DISCONNECT) {
        INFO("disconnect fd=%d\n", conn->fd);

        shutdown(conn->fd, SHUT_WR);
        conn_free(conn);
        return;
    }

    if (what & EV2_READABLE) {
        while (1) {
            len = recv(conn->fd, buf, sizeof(buf), MSG_DONTWAIT);
            if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;

                perror("recv");
                conn_free(conn);
                return;
            }
            else if (len == 0) {
                INFO("disconnect fd=%d\n", conn->fd);
	         	printf("连接fd=%d转发的数据量为%d Bytes\n",fd,conn->bytes);//打印每条连接数据
                shutdown(conn->fd, SHUT_WR);
                conn_free(conn);
                return;
            }
            else {
                err = simple_send(conn->fd2, buf, (size_t)len);
                if (err < 0) {
                    conn_free(conn);
                    return;
                }
				conn->bytes+=len;
                if (len < (int)ARRAY_SIZE(buf) ||
                    count++ >= 1) {
                    break;
                }
            }
        }
    }
}

static void echo_server__on_readable2(void *arg, int fd, int what)//++
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

    if (what & EV2_DISCONNECT) {
        INFO("disconnect fd=%d\n", conn->fd);

        shutdown(conn->fd, SHUT_WR);
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
                INFO("disconnect fd=%d\n", conn->fd);
	         	printf("连接fd=%d转发的数据量为%d Bytes\n",fd,conn->bytes);//打印每条连接数据
                shutdown(conn->fd, SHUT_WR);
                conn_free(conn);
                return;
            }
            else {
                err = simple_send(conn->fd, buf, (size_t)len);
                if (err < 0) {
                    conn_free(conn);
                    return;
                }
				conn->bytes+=len;
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
	memset(&sa2,0,sizeof(sa2));
	sa2.sin6_family=AF_INET6;
	sa2.sin6_addr.__in6_u.__u6_addr32[4]=inet_addr("192.168.91.128");
	sa2.sin6_port=htons(8080);

    socklen_t sa_len;
    int cfd;
    int err;

    if (what < 0) {
        echo_server_free(s);
        return;
    }

    if (what & EV2_READABLE) {
        memset(&sa, 0, sizeof(struct sockaddr_in6));
        sa_len = sizeof(struct sockaddr_in6);
        cfd = accept(s->fd, &sa, &sa_len);
        if (cfd < 0) {
            perror("accept");
            if (errno == EMFILE ||
                errno == EAGAIN) {
                return;
            }
            echo_server_free(s);
            return;
        }
	
		char dst[64];
		inet_ntop(AF_INET6,&sa.sin6_addr.s6_addr,dst,sizeof(dst));
		printf("The connection IP=%s,Port=%d\n",dst,ntohs(sa.sin6_port));

		
        conn_t *conn = conn_new(s->loop, cfd);
        if (conn == NULL) {
            close(cfd);
            return;
        }


		conn->fd2=socket(AF_INET6,SOCK_STREAM,0);
		if(conn->fd2<0)
		{
			perror("socket error");
			abort();	
		}

		int con=connect(conn->fd2,(struct sockaddr*)&sa2,sizeof(sa2));//++
		if(con<0)
		{
			perror("connection error");
			abort();
		}
		
		
        err = ev2_poll_register(conn->poll, cfd, EV2_READABLE, conn, echo_server__on_readable);
        if (err < 0) {
            //perror("epoll_ctl");
            conn_free(conn);
        }
	
		err =0;//++
		err = ev2_poll_register(conn->poll,conn->fd2,EV2_READABLE,conn,echo_server__on_readable2);//++
		if(err < 0)//++
		{
			conn_free(conn);
		}

        printf("new connection fd=%d\n", conn->fd);
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
    s->fd = socket(PF_INET6, SOCK_STREAM, 0);
    if (s->fd < 0) {
        perror("socket");
        abort();
    }
    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   // setsockopt(s->fd, IPPROTO_IPV6, IPV6_V6ONLY,&opt,sizeof(opt));禁用IPV4

    memset(&sa, 0, sizeof(struct sockaddr_in6));
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons((uint16_t)port);

    err = bind(s->fd, (struct sockaddr *)&sa, sizeof(sa));
    if (err < 0) {
        close(s->fd);
        s->fd = -1;
        perror("bind");
        return -1;
    }

    err = listen(s->fd, 100);
    if (err < 0) {
        close(s->fd);
        s->fd = -1;
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
