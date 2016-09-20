#include "threadpool.h"
#include "utility.h"
#include "cache.h"

#define KEY_SZ 100
#define VALUE_SZ 1000


LRUCache cache(1000);
pthread_mutex_t cache_mutex;
bool re = pthread_mutex_init(&cache_mutex, NULL);

void *worker(void *arg)
{
	int clientfd = atoi((char*)arg);
    char buf[BUF_SIZE], message[BUF_SIZE];

    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

	int len = recv(clientfd, buf, sizeof(buf), 0);
	printf("recv len=%d\n", len);
	if(len == 0)
	{
		close(clientfd);
		clients_list.remove(clientfd);
		printf("client_%d closed.\n %d client exits\n", clientfd, (int)clients_list.size());
	}
	/*
	int rs = 1;
	while(rs)
	{
		int len = recv(clientfd, buf, sizeof(buf), 0);
		printf(":while...rs=%d\n",rs);
		if(len < 0)
		{
			if(errno == EAGAIN) continue;
			else exit(-1);
		}
		else if(len == 0)
		{
			close(clientfd);
			clients_list.remove(clientfd);
			printf("client_%d closed.\n %d client exits\n", clientfd, (int)clients_list.size());
			break;
		}
		if(len == sizeof(buf)) rs = 1;
		else rs = 0;	
	}
	*/
	else
	{
		pthread_mutex_lock(&cache_mutex);

		if(strncasecmp(buf, "set", 3) == 0)
		{
			printf("set...\n");
			string key,value;
			int key_sz = 0, value_sz = 0, i = 3;

			for(; buf[i] != '\0'; ++i)
				if(buf[i] != ' ') break;
			for(; buf[i] != '\0' && buf[i] != ' '; ++i)
				key += buf[i];

			for(; buf[i] != '\0'; ++i)
				if(buf[i] != ' ') break;
			for(; buf[i] != '\0' && buf[i] != ' '; ++i)
				value += buf[i];

			cache.Set(key,value);
			sprintf(message, "set succ!");
		}
		else if(strncasecmp(buf, "get", 3) == 0)
		{
			printf("get...\n");
			string key;
			int i = 3;
			for(; buf[i] != '\0'; ++i)
				if(buf[i] != ' ') break;
			for(; buf[i] != '\0'; ++i)
				key += buf[i];

			strcpy(message, (cache.Get(key)).c_str());
		}
		else
		{
			printf("else...\n");
			sprintf(message,"please retry!(usage:get/set)");
		}

		pthread_mutex_unlock(&cache_mutex);

		if( send(clientfd, message, sizeof(message), 0) < 0 ) 
		{ 
			perror("send() error"); 
			exit(-1);
		}
	}
	sleep(1);
}

int main(int argc, char *argv[])
{
	struct threadpool *pool = threadpool_init(10, 20);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
	int listener = socket(PF_INET, SOCK_STREAM, 0);
    if (listener < 0)
	{
		perror("socket() error!");
		exit(-1);
	}
    ///printf("listen socket create succ! \n");
    
	if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
	{
        perror("bind() error!");
        exit(-1);
    }
    
	int ret = listen(listener, 100);
    if(ret < 0)
	{
		perror("listen() error!");
		exit(-1);
	}
    ///printf("Start to listen: %s ...\n", SERVER_IP);
    
	int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
	{
	   	perror("epoll_create() error!");
		exit(-1);
	}
    ///printf("epoll create succ! epollfd = %d\n", epfd);
	
    static struct epoll_event events[EPOLL_SIZE];
    addfd(epfd, listener, true);
    
	while(true)
    {
        int num_epoll_events = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(num_epoll_events < 0) 
		{
            perror("epoll_wait() fail!");
            continue;
        }

        ///printf("num_epoll_events = %d\n", num_epoll_events);
        for(int i = 0; i < num_epoll_events; ++i)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept( listener, ( struct sockaddr* )&client_address, &client_addrLength );
				if(clientfd < 0)
				{
					perror("accept() erroe!");
					continue;
				}
				printf("client connection from: %s:% d(IP : port), clientfd = %d \n",
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);

                addfd(epfd, clientfd, true);
				
                clients_list.push_back(clientfd);
                printf("Add new clientfd = %d to epoll\n", clientfd);
                printf("Now there are %d clients int the list\n", (int)clients_list.size());
				
				
				char message[BUF_SIZE];
				bzero(message, BUF_SIZE);
				sprintf(message, "This is server,send msg to client_%d", clientfd);
				int ret = send(clientfd, message, BUF_SIZE, 0);
				if(ret < 0) 
				{ 
					perror("send() error!"); 
					exit(-1); 
				}
			
            }
			else if(events[i].events & EPOLLIN)
			{
				printf("epollin...\n\n");
				char c_sockfd[20];
				snprintf(c_sockfd, sizeof(c_sockfd), "%d", sockfd);  	
				threadpool_add_job(pool, worker, c_sockfd);
			}
		}
    }
    close(listener);
    close(epfd);

	sleep(5);
    threadpool_destory(pool);
    return 0;
}
