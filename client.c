//this client code, refer to below code

// Created by Injae Lee on 2014. 10. 10. Modified by Minsuk Lee,
// Copyright (c) 2014. Injae Lee All rights reserved.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define IP "127.0.0.1"
#define PORT 3000
#define BUF_SIZE 100
#define EPOLL_SIZE 30

int main()
{
    int ret = -1;
    int clientSock, wsize;
    struct sockaddr_in serverAddr;
    char buf[BUF_SIZE];

    struct epoll_event *epollEvents;
    struct epoll_event event;
    int epoolFd, eventCount, byteLength;    



    if ((clientSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto leave;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    if ((ret = connect(clientSock,(struct sockaddr*)&serverAddr,
                       sizeof(serverAddr)))) {
        perror("connect");
        goto error;
    }

    //set epoll
    epoolFd = epoll_create(EPOLL_SIZE);
    epollEvents = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = 0;
    epoll_ctl(epoolFd, EPOLL_CTL_ADD, 0, &event);
    event.data.fd = clientSock;
    epoll_ctl(epoolFd, EPOLL_CTL_ADD, clientSock, &event);

    while(1) {

        eventCount = epoll_wait(epoolFd, epollEvents, EPOLL_SIZE, -1);
        if(eventCount == -1) {
            fprintf(stderr, "epoll_wait() error");
            break;
        }

        for(int i = 0; i < eventCount; i++) { 
            if(epollEvents[i].data.fd == 0) {
                fgets(buf, BUF_SIZE, stdin);
                if ((wsize = send(clientSock, buf, strlen(buf), 0)) <= 0) {
                    perror("send");
                    //fprintf(stderr, "error: %s", "send");
                }
            } else if(epollEvents[i].data.fd == clientSock) {
                byteLength = read(clientSock, buf, BUF_SIZE - 1);//epollEvents[i].data.fd == clientSock
                if(byteLength == 0) {
                    epoll_ctl(epoolFd, EPOLL_CTL_DEL, clientSock, NULL);
                    close(clientSock);
                    goto leave;
                } else {
                    buf[byteLength] = '\0'; //convert string
                    printf("%s\n",buf);
                }          
            }          
        }
    }

error:
    close(clientSock);
leave:
    return ret;
}

