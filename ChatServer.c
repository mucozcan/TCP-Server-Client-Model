/*
A server-side code of simple TCP Chat App.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/
            https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872

Usage: ./ChatServer [The port number that you want]

        in another terminal: ./ChatClient [Port number that you defined.]

Server does all send and receive functions in multithread function sendAndReceive that defined in Chat.h
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
    //sockaddr is a structure for handling internet addresses.
    struct sockaddr_in serverAddr;
    int portNo, serverSocket;
    portNo = atoi(argv[1]);
    serverSocket = createIPv4Socket();
    //Defining server's address information.
    serverAddr = defineSocket(portNo); //passing port number that taken as parameter.
    bindSocket(serverSocket,serverAddr); //binding socket
    listenConnections(serverSocket,MAX_CLIENT); //listening incoming connections.
    printf("Server started listening on port %d.\n", portNo);
    while (1)
    {
        //assigning new socket descriptor to client's sockID
        Client[clientCount].sockID = acceptConnection(serverSocket,&Client[clientCount],clientCount);
        Client[clientCount].index = clientCount;

        //creating thread for client
        pthread_create(&thread[clientCount], NULL, sendAndReceive, (void *)&Client[clientCount]);
        clientCount++;
    }
} //end main()