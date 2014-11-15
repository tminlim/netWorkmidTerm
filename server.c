#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15
#endif

#define BUF_SIZE 100
#define EPOLL_SIZE 30
#define FDLIST_SIZE 10

struct fdList {
	int fd[FDLIST_SIZE];
};

void error_handling(char *buf);
void addFdList(struct fdList * list, int fd);
void removeFdList(struct fdList * list, int fd);
void broadcast(struct fdList * list, int fd, char *buf, int length);

int main (int argc, char* argv[]) {
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t addrLength;

	struct epoll_event *epollEvents;
	struct epoll_event event;
	int epoolFd, eventCount, byteLength, wSize;
	char buf[BUF_SIZE];
	struct fdList list;

	if(argc != 2) {
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}

	memset(&list, 0, sizeof(list));

	//open socket
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(atoi(argv[1]));
	
	//bind
	if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		error_handling("bind error");
	
	//listen
	if(listen(serverSocket, 5) == -1)
		error_handling("listen error");

	//set epoll
	epoolFd = epoll_create(EPOLL_SIZE);
	epollEvents = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	event.events = EPOLLIN;
 	event.data.fd = 0;
     epoll_ctl(epoolFd, EPOLL_CTL_ADD, 0, &event);
	event.data.fd = serverSocket;
	epoll_ctl(epoolFd, EPOLL_CTL_ADD, serverSocket, &event);


	while(1) {
		eventCount = epoll_wait(epoolFd, epollEvents, EPOLL_SIZE, -1);
		if(eventCount == -1) {
			fprintf(stderr, "epoll_wait() error");
			break;
		}
	//accept
	//read n write		

		for(int i = 0; i < eventCount; i++) {
			if(epollEvents[i].data.fd == serverSocket) {
				addrLength = sizeof(clientAddress);
				clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addrLength);
				event.events = EPOLLIN;
				event.data.fd = clientSocket;
				epoll_ctl(epoolFd, EPOLL_CTL_ADD, clientSocket, &event);
				addFdList(&list, clientSocket);
				printf("connect client %d\n", clientSocket);
			} else if (epollEvents[i].data.fd == 0) {
	                fgets(buf, BUF_SIZE, stdin);
				 broadcast(&list, 0, buf, strlen(buf));
              } else {
				byteLength = read(epollEvents[i].data.fd, buf, BUF_SIZE - 1);
				if(byteLength == 0) {
					epoll_ctl(epoolFd, EPOLL_CTL_DEL, epollEvents[i].data.fd, NULL);
					close(epollEvents[i].data.fd);
					//printf("close client! %d \n", epollEvents[i].data.fd );
				} else {
					buf[byteLength] = '\0'; //convert string
					printf("%s\n",buf);
					//write(epollEvents[i].data.fd, buf, byteLength);
					broadcast(&list, epollEvents[i].data.fd, buf, byteLength);
				}
			}
		}
	}

	//close
	close(serverSocket);
	close(epoolFd);
	return 0;
}

void addFdList(struct fdList * list, int fd) {
	for (int idx = 0; idx < FDLIST_SIZE; idx++) {
		if(list->fd[idx] == 0) {
			list->fd[idx] = fd;
			break;
		} 
	}
}

void removeFdList(struct fdList * list, int fd) {
	for (int idx = 0; idx < FDLIST_SIZE; idx++) {
		if(list->fd[idx] == fd) {
			list->fd[idx] = 0;
		}
	}
}

void broadcast(struct fdList * list, int fd, char *buf, int length) {
	for(int idx = 0; idx < FDLIST_SIZE; idx++) {
		if(list->fd[idx] == fd)
			continue;

		if(list->fd[idx] == 0)
			continue;

		if (send(list->fd[idx], buf, length, 0) <= 0) {
	          perror("send");
	          //fprintf(stderr, "error: %s", "send");
	     }
	}
}

void error_handling(char *buf) {
	fprintf(stderr, "%s\n", buf);
	exit(1);
}