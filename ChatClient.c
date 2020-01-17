/*
A client-side code of simple TCP Chat App.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/
            https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872

Usage: ./ChatServer [The port number that you want]

        in another terminal: ./ChatClient [Port number that you defined.] [username] [password]
		>LIST
		*List of all connected clients.
		>SEND <ID> <MESSAGE>
		*Send <MESSAGE> to Client<ID>.
		>EXIT
		*Disconnected to server and exit program.
Client does all receive functions in multithread function receiveMessages that defined in Chat.h

*/


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
#include <signal.h>
#include "Chat.h"

static volatile int keepRunning = 1;
static int clientSocket;
static int authSocket;

void quitHandler(int client)
{
    keepRunning = 0;
    close(clientSocket); //closing socket.
	char logString[256];
    printf("\nSocket: %d is closed\nClosing application... Operations and errors logged.\n",clientSocket);
    sprintf(logString,"Socket: %d is closed, Closing application...",clientSocket);
    logOperations(logString);
    exit(1);
}
int main(int argc, char **argv)
{
	signal(SIGINT,quitHandler); //closing socket and quit.

	struct sockaddr_in serverAddr;//for communication.
	struct sockaddr_in authAddr; //for authentication
	authSocket = createIPv4Socket();
	authAddr = defineSocket(atoi(argv[1]));
	connectToServer(authSocket,authAddr); //first connects to the auth. port and sends client's info.
	char *username = argv[2];
	char *password = argv[3];
	char userInfo[1024];
	char serverPortNum[5]; //com port
	sprintf(userInfo, "%s %s",username,password);
	send(authSocket,userInfo,1024,0);
	printf("\nReceiving..\n");
	int receivedPort = recv(authSocket,serverPortNum,1024,0); //if user is valid server sends port number for communication.
	int serverPort = atoi(serverPortNum);
	if(serverPort == 0){
		printf("\nYou are not a valid user\n");
		exit(1);
	}
	printf("\nClosing authentication socket and routing to communication socket...\n");
	close(authSocket);
	authSocket = -1;
	printf("\nServer port: %d\n",serverPort);
	clientSocket = createIPv4Socket();
	serverAddr = defineSocket(serverPort);
	connectToServer(clientSocket,serverAddr); //connects to communication socket.


	pthread_t thread;
	pthread_create(&thread, NULL, receiveMessages, (void *) &clientSocket );

	while(keepRunning){

		char input[1024];
		scanf("%s",input);
		if(strcmp(input, "EXIT") == 0)
		{
			break;
		}

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