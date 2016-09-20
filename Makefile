server : epoll_server.cpp threadpool.o epoll_client.cpp
	g++ -std=c++11 threadpool.o epoll_client.cpp -o client -lpthread
	g++ -std=c++11 threadpool.o epoll_server.cpp -o server -lpthread

threadpool.o : threadpool.cpp
	g++ -c threadpool.cpp -lpthread

clean :
	rm -rf  *.o


