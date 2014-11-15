#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

void error_handling(char *buf);

int main (int argc, char* argv[]) {
	int serverSocket;
	struct sockaddr_in serverAddress;

	//open socket
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htonl(atoi(argv[1]));
	
	//bind
	if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		error_handling("bind error");
	
	//listen
	if(listen(serverSocket, 5) == -1)
		error_handling("listen error");
	//accept
	//read n write
	//close
}

void error_handling(char *buf) {
	fprintf(stderr, "%s\n", buf);
	exit(1);
}