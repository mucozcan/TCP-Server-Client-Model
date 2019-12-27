/*
A client-side code of simple TCP Chat App.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/
            https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872

Usage: ./ChatServer [The port number that you want]

        in another terminal: ./ChatClient [Port number that you defined.]
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
#include "Chat.h"


int main(int argc, char **argv)
{
	struct sockaddr_in serverAddr;
	int clientSocket;
	clientSocket = createIPv4Socket();
	serverAddr = defineSocket(atoi(argv[1]));
	connectToServer(clientSocket,serverAddr);

	pthread_t thread;
	pthread_create(&thread, NULL, receiveMessages, (void *) &clientSocket );

	while(1){

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
	printf("Disconnected with server.\n");
	close(clientSocket);
}