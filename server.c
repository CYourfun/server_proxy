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
#define Port 8000

int main(){
    int lfd = socket(AF_INET, SOCK_STREAM,0);//(1)socket
	if(lfd<0)
	{
		perror("socket error");	
	}
    struct sockaddr_in serv_addr;//serv_addr
	struct sockaddr_in cli_addr;//cli_addr
	memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET; //IPV4 
    serv_addr.sin_addr.s_addr =INADDR_ANY; //inet_addr(IP)  
    serv_addr.sin_port = htons(Port); 
	//设置端口复用
	int opt=1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));
	signal(SIGPIPE, SIG_IGN);//忽略所有信号
	struct rlimit rli;
	
	if(getrlimit(RLIMIT_NOFILE,&rli)==0)//细节处理
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
	//增加一行注释
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

	int ret= bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));//(2)bind
    if(ret<0)
	{
		perror("bind error");
		exit(1);
	}
	int li=listen(lfd, 100);//(3)listen
	if(li<0)
	{
		perror("listen error");
		exit(1);
	}
	//epoll
	struct epoll_event events[1024],event;
	int epfd=epoll_create(256);//I: epoll_create
	if(epfd<0)
	{
		perror("epoll create fail");
		exit(1);	
	}

	//将lfd挂到epoll对象上并注册监听事件
	event.events=EPOLLIN|EPOLLHUP|EPOLLRDHUP|EPOLLERR;
	event.data.fd=lfd;
	int ctl=epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&event);//II: epoll_ctl
	if(ctl<0)
	{
		perror("epoll_ctl fail");	
		exit(1);
	}
		
	
	char buf[4096];

	//将lfd设置为非阻塞
	int lflags=fcntl(lfd,F_GETFL);
	lflags|=O_NONBLOCK;	
	fcntl(lfd,F_SETFL,lflags);

	while(1){
		int eret=epoll_wait(epfd,events,1024,-1);//III: epoll_wait
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
		    	if(events[i].events & EPOLLIN)
			    {
					if(events[i].data.fd==lfd) 
					{		
    					socklen_t cli_addr_size = sizeof(cli_addr);
						char dst[64];
    				    int cfd = accept(lfd, (struct sockaddr*)&cli_addr, &cli_addr_size);//(4)accept
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
				
					//更改文件描述符cfd为非阻塞
						int flags=fcntl(cfd,F_GETFL);
						flags|=O_NONBLOCK;
						fcntl(cfd,F_SETFL,flags);			

						inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,dst,sizeof(dst));
				//	printf("The connection IP=%s,Port=%d\n",dst,ntohs(cli_addr.sin_port));
						event.events=EPOLLIN;// | EPOLLET;//ET触发
						event.data.fd=cfd;
						epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&event);				
					}
					else
			 		{
						memset(buf,0,sizeof(buf));//验证是否需要清空buf
						while(1)
						{
							int rr=read(events[i].data.fd,buf,sizeof(buf));
						
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
								close(events[i].data.fd);
								epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
								break;	
							}
							else if(rr==0)
							{
						//	printf("客户端断开连接\n");
								close(events[i].data.fd);
								epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
								break;
							}
							else if(rr>0)
							{
								int ww = 0;
								int write_count = 0;
							//printf("读取到的字节数是:%d\n",rr);
								while(1)
								{
									int r = write(events[i].data.fd, buf + ww, rr - ww);//疑问，buf是否需要memset?
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
										printf("fd:%d write_count: %d\n",events[i].data.fd,write_count);
										}
										break;
									}
									printf("fd=%d, write_count=%d, rr=%d, ww=%d\n",
									events[i].data.fd,write_count, rr, ww);
								}								
							}
		
						}
						break;//跳出读循环，防止一直read，read速度快，write速度慢，导致其他连接进不来超时
					} 
		    	}
			}
		}	   
	}
    return 0;
}
