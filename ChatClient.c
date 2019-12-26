#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void *doReceiving(void *sockID)
{
	int clientSocket = *((int *)sockID);

	while (1)
	{
		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';
		printf("%s\n", data);
	}
}

int main(int argc, char **argv)
{

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1){
		perror("connect:");
		return 0;
	}

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, doReceiving, (void *) &clientSocket );

	while(1){

		char input[1024];
		scanf("%s",input);

		if(strcmp(input,"LIST") == 0){

			send(clientSocket,input,1024,0); //send LIST command to server.

		}
		if(strcmp(input,"SEND") == 0){

			send(clientSocket,input,1024,0); //send SEND command to server.
			
			scanf("%s",input);
			send(clientSocket,input,1024,0); //send ID of other client to server for communicating.
			
			scanf("%[^\n]s",input);	
			send(clientSocket,input,1024,0); //send message to server.

		}

	}
}