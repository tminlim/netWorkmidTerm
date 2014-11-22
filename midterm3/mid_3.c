#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define MIN_BUF 100

void* requestHandler(void* argu);
void sendData(FILE* fp, char* contentType, char* fileName);
//char*
const char* content_type(const char* file);
void sendError(FILE* fp);
void errorHandling(char* message);

int main(int argc, char *argv[]){
	int serverSocket, clientSocket;
	//unchangeable?
	struct sockaddr_in serv_adr, clnt_adr;
	int lengthClientAddress;
	char buf[BUF_SIZE];
	pthread_t t_id;
	if(argc != 2){
		printf("usage: %s <port>\n", argv[0]);
		exit(1);
	} 
	
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serverSocket, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==1)
		errorHandling("bind error");
    
	if(listen(serverSocket, 12)==-1)
		errorHandling("listen error");
	
	while(1){
		lengthClientAddress = sizeof(clnt_adr);
		clientSocket = accept(serverSocket, (struct sockaddr*)&clnt_adr, &lengthClientAddress);
		printf("connect request! :%s:%d\n", inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
		pthread_create(&t_id, NULL, requestHandler, &clientSocket);
		pthread_detach(t_id);
	}
	close(serverSocket);
	return 0;
}

void* requestHandler(void* argu){
	int clientSocket =*((int*)argu);
	char requestLine[MIN_BUF];
	FILE* clientWrite;
	FILE* clientRead;

	char method[10];
	char contentType[15];
	char fileName[30];

	clientRead = fdopen(clientSocket, "r");
	clientWrite = fdopen(clientSocket, "w");
	fgets(requestLine, MIN_BUF, clientRead);

	if(strstr(requestLine, "HTTP/") == NULL){
		sendError(clientWrite);
		fclose(clientRead);
		fclose(clientWrite);
		return NULL;
	}

	strcpy(method, strtok(requestLine, " /"));
	strcpy(fileName, strtok(NULL, " /"));
	strcpy(contentType, content_type(fileName));
	if(strcmp(method, "GET") != 0){
		sendError(clientWrite);
		fclose(clientRead);
		fclose(clientWrite);
		return NULL;
	}
	
	fclose(clientRead);
	sendData(clientWrite, contentType, fileName);
    
    return NULL;
}

void sendData(FILE* fp, char* contentType, char* fileName){
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char server[] = "Server:linux web server \r\n";
	char contentLength[] = "content length:2048\r\n";
	char arrContentType[MIN_BUF];
	char buf[BUF_SIZE];
	FILE* sendFile;

	sprintf(contentType, "contentType:%s\r\n\r\n", contentType);
	sendFile = fopen(fileName,  "r");
	if(sendFile == NULL){
		sendError(fp);
		return;
	}

	/* send header info*/
	fputs(protocol, fp);
	fputs(server, fp);
	fputs(contentLength, fp);
	fputs(contentType, fp);

	/*send data of request*/
	while(fgets(buf, BUF_SIZE, sendFile) != NULL){
		fputs(buf, fp);
		fflush(fp);
	}
	fflush(fp);
	fclose(fp);
}

const char* content_type(const char* file){
	char extension[MIN_BUF];
	char fileName[MIN_BUF];
	strcpy(fileName, file);
	strtok(fileName, ".");
	strcpy(extension, strtok(NULL, "."));

	if(!strcmp(extension, "html")|| strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void sendError(FILE* fp){
	char protocol[] = "HTTP/1.0 400 bad request\r\n";
	char server[] = "server:linux werb server \r\n";
	char contentLength[] = "content length:2048\r\n";
	char contentType[] = "content type:text/html\r\n\r\n";
	char content[] = "<html><head><title> network </title></head>"
	"<body><font-size = +5>error occur! check the content type & file name"
	"</font></bodly></html>";

	fputs(protocol, fp);
	fputs(server, fp);
	fputs(contentLength, fp);
	fputs(contentType, fp);
	fflush(fp);
}

void errorHandling(char* message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

