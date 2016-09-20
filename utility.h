#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

list<int> clients_list;

#define SERVER_IP "127.0.0.1"

#define SERVER_PORT 8888

#define EPOLL_SIZE 5000

#define BUF_SIZE 0xFFFF

#define EXIT "EXIT"

int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)| O_NONBLOCK);
    return 0;
}

void addfd( int epollfd, int fd, bool is_et )
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if( is_et )
	{
        ev.events = EPOLLIN | EPOLLET;
	}
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
    ///printf("fd_%d added to epoll succ!\n\n",fd);
}

#endif 
