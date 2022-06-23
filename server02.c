#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<sys/epoll.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include <sys/resource.h>
//#define Port 8000


void setNonblocking(int fd)
{
	//将fd设置为非阻塞
	int lflags=fcntl(fd,F_GETFL);
	lflags|=O_NONBLOCK;	
	fcntl(fd,F_SETFL,lflags);
}


struct echo_server_s
{
	int fd;
	struct sockaddr_in ser_addr;
	struct sockaddr_in cli_addr;
};


void echo_server_new(struct echo_server_s *s)
{
    s->fd = socket(AF_INET,SOCK_STREAM,0);//(1)socket
	if(s->fd<0)
	{
		perror("socket error");	
	}

}

void echo_server_listen(struct echo_server_s *s,int port)
{
    //struct sockaddr_in serv_addr;//serv_addr
	//struct sockaddr_in cli_addr;//cli_addr
	memset(&s->ser_addr, 0, sizeof(s->ser_addr));  
   s->ser_addr.sin_family = AF_INET; //IPV4 
   s->ser_addr.sin_addr.s_addr =INADDR_ANY; //inet_addr(IP)    
			
	s->ser_addr.sin_port = htons(port);//Port

	//设置端口复用
	int opt=1;
	setsockopt(s->fd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));
	signal(SIGPIPE, SIG_IGN);//忽略所有信号

	struct rlimit rli;
	if(getrlimit(RLIMIT_NOFILE,&rli)==0)//细节处理,设置系统最大文件打开数
	{
		printf("rlim_max:%ld rlim_cur=%ld\n",rli.rlim_max,rli.rlim_cur);
		if(rli.rlim_max>65536)
		{
			rli.rlim_cur=65536;
		}
		else
		{
			rli.rlim_cur=rli.rlim_max;
		}
	//	rli.rlim_cur=65535;
		int srlim=setrlimit(RLIMIT_NOFILE,&rli);
		if(srlim<0)
		{
			if(errno==EPERM)
			{
				perror("权能不允许\n");	
			}
			else if(errno==EINVAL)
			{
				perror("参数无效\n");	
			}
			else if(errno==EFAULT)
			{
				perror("rlim指向空间不可访问\n");
			}
			perror("setrlim error");	
		}	
	}
	int ret= bind(s->fd,(struct sockaddr*)&s->ser_addr,sizeof(s->ser_addr));//(2)bind
    if(ret<0)
	{
		perror("bind error");
		exit(1);
	}
	int li=listen(s->fd,100);//(3)listen
	if(li<0)
	{
		perror("listen error");
		exit(1);
	}
}

struct epoll_s
{
	int epfd;
	struct epoll_event event,events[1024];
    int fd;

};

void epoll_new(struct echo_server_s *s,struct epoll_s *loop)
{
	//epoll
	//struct epoll_event event;
	loop->epfd=epoll_create(256);//I: epoll_create
	if(loop->epfd<0)
	{
		perror("epoll create fail");
		exit(1);	
	}

	//将lfd挂到epoll对象上并注册监听事件
	loop->event.events=EPOLLIN|EPOLLHUP|EPOLLRDHUP|EPOLLERR;
	loop->event.data.fd=s->fd;
	int ctl=epoll_ctl(loop->epfd,EPOLL_CTL_ADD,s->fd,&loop->event);//II: epoll_ctl
	if(ctl<0)
	{
		perror("epoll_ctl fail");	
		exit(1);
	}
}

void epoll_run(struct epoll_s *loop,struct echo_server_s *s)
{
	char buf[4096];
//	struct epoll_event events[1024],event;
	setNonblocking(s->fd);//lfd设置为非阻塞
	while(1){
		int eret=epoll_wait(loop->epfd,loop->events,1024,-1);//III: epoll_wait
		if(eret<0)
	    {
	    	perror("epoll error");
			exit(1);
	    }
	    if(eret>0)
	    {
	   	    int i;
			for(i=0;i<eret;i++)
		    {
		    	if(loop->events[i].events & EPOLLIN)
			    {
					if(loop->events[i].data.fd==s->fd) 
					{		
    					socklen_t cli_addr_size = sizeof(s->cli_addr);
						//char dst[64];存ip地址和端口号
    				    int cfd = accept(s->fd, (struct sockaddr*)&s->cli_addr, &cli_addr_size);//(4)accept
						if(cfd<0)//思考
						{
							if(errno==EWOULDBLOCK)
							{
								continue;
							}
							perror("accept error");
							exit(-1);
						}
					//printf("客户端连接建立成功cfd=%d\n",cfd);
				
					//更改文件描述符cfd为非阻塞（未完成）

					//	inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,dst,sizeof(dst));
				//	printf("The connection IP=%s,Port=%d\n",dst,ntohs(cli_addr.sin_port));
						loop->event.events=EPOLLIN;// | EPOLLET;//ET触发
						loop->event.data.fd=cfd;
						epoll_ctl(loop->epfd,EPOLL_CTL_ADD,cfd,&loop->event);				
					}
					else
			 		{
						memset(buf,0,sizeof(buf));//验证是否需要清空buf
						while(1)
						{
							int rr=read(loop->events[i].data.fd,buf,sizeof(buf));
						
							if(rr<0)
							{
								if(errno==EAGAIN)//以非阻塞方式读取socket且当前socket为空
								{	
									break;	
								}
							
								if(errno==EBADF)
								{
									perror("文件描述符未打开");
									break;	
								}
								perror("read error");
								close(loop->events[i].data.fd);
								epoll_ctl(loop->epfd,EPOLL_CTL_DEL,loop->events[i].data.fd,NULL);
								break;	
							}
							else if(rr==0)
							{
						//	printf("客户端断开连接\n");
								close(loop->events[i].data.fd);
								epoll_ctl(loop->epfd,EPOLL_CTL_DEL,loop->events[i].data.fd,NULL);
								break;
							}
							else if(rr>0)
							{
								int ww = 0;
								int write_count = 0;
							//printf("读取到的字节数是:%d\n",rr);
								while(1)
								{
									int r = write(loop->events[i].data.fd, buf + ww, rr - ww);//疑问，buf是否需要memset?
									write_count += 1;
									if (r < 0)
									{
										if(errno==EAGAIN)
										{
											usleep(1000);//write太快了，等一会
											continue;
									//	perror("write error");
									//		break;
										}
										if(errno==EINTR)
										{
											perror("信号中断\n");
											continue;	
										}
										perror("write error");
									//close(events[i].data.fd);
									//epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
										break;	
									}
									ww += r;
									if(rr==ww)
									{
										if (write_count > 1) {
										printf("fd:%d write_count: %d\n",loop->events[i].data.fd,write_count);
										}
										break;
									}
									printf("fd=%d, write_count=%d, rr=%d, ww=%d\n",
									loop->events[i].data.fd,write_count, rr, ww);
								}								
							}
		
						}
						break;//跳出读循环，防止一直read，read速度快，write速度慢，导致其他连接进不来超时
					} 
		    	}
			}
		}	   
	}

}


int main(int argc,char* argv[]){	
	struct echo_server_s *s1=(struct echo_server_s *)malloc(sizeof(struct echo_server_s));
	echo_server_new(s1);
	echo_server_listen(s1,8000);
    struct epoll_s *loop=(struct epoll_s *)malloc(sizeof(struct epoll_s));
	epoll_new(s1,loop);
	epoll_run(loop,s1);
	
	free(s1);
	free(loop);

	

    return 0;
}
