#include "utility.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) 
	{ 
		perror("socket() error"); 
		exit(-1); 
	}
    if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
	{
        perror("connect() error!");
        exit(-1);
    }

    ///创建管道，fd[0]用于父进程读，fd[1]用于子进程写
    int pipe_fd[2];
    if(pipe(pipe_fd) < 0) 
	{ 
		perror("pipe() error!"); 
		exit(-1); 
	}

    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0) 
	{ 
		perror("epoll_create() error!"); 
		exit(-1); 
	}
    static struct epoll_event events[2];
    addfd(epfd, sock, true);
    addfd(epfd, pipe_fd[0], true);
    
	bool isClientwork = true;///表示客户端是否正常工作

    char message[BUF_SIZE];

    int pid = fork();
    if(pid < 0) 
	{ 
		perror("fork() error!"); 
		exit(-1); 
	}
    else if(pid == 0)///子进程
    {
        ///子进程负责写入管道，关闭读端
        close(pipe_fd[0]);
        ///printf("'exit' to EXIT.\n");

        while(isClientwork)
		{
            bzero(&message, BUF_SIZE);
            fgets(message, BUF_SIZE, stdin);

            if(strncasecmp(message, EXIT, strlen(EXIT)) == 0)
			{
                isClientwork = 0;
            }
            else 
			{
                if( write(pipe_fd[1], message, strlen(message) - 1 ) < 0 )
                {
				   	perror("fork_son write() error!"); 
					exit(-1); 
				}
            }
        }
    }
    else ///pid > 0 父进程
    {
        //父进程负责读管道数据，关闭写端
        close(pipe_fd[1]);

        while(isClientwork) 
		{
            int num_epoll_events = epoll_wait( epfd, events, 2, -1 );
            for(int i = 0; i < num_epoll_events ; ++i)
            {
                bzero(&message, BUF_SIZE);
                if(events[i].data.fd == sock)//服务端发来消息
                {
                    int ret = recv(sock, message, BUF_SIZE, 0);
                    if(ret == 0) 
					{
                        printf("Server closed connection: %d\n", sock);
                        close(sock);
                        isClientwork = 0;
                    }
                    else printf("%s\n", message);

                }
                //子进程写入事件发生，父进程处理并发送服务端
                else
			   	{
                    //父进程从管道中读取数据
                    int ret = read(events[i].data.fd, message, BUF_SIZE);

                    if(ret == 0) 
					{
						isClientwork = 0;
					}
                    else
					{   // 将信息发送给服务端
                    	send(sock, message, BUF_SIZE, 0);
                    }
                }
            }
        }
    }

    if(pid)
	{
        close(pipe_fd[0]);
        close(sock);
    }
	else
	{
        close(pipe_fd[1]);
    }
    return 0;
}
