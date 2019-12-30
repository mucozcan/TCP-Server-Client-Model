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
#include <signal.h>
#include "Chat.h"

static volatile int keepRunning = 1;
static int serverSocket;

void quitHandler(int server)
{
    keepRunning = 0;
    close(serverSocket); //closing socket.
    char logString[256];
    printf("\nSocket: %d is closed\nClosing application... Operations and errors logged.\n",serverSocket);
    sprintf(logString,"Socket: %d is closed, Closing application...",serverSocket);
    logOperations(logString);
    exit(1);
}

int main(int argc, char **argv)
{
    signal(SIGINT,quitHandler);
    //sockaddr_in is a structure for handling internet addresses.
    struct sockaddr_in serverAddr;
    int portNo;
    portNo = atoi(argv[1]);
    serverSocket = createIPv4Socket();
    //Defining server's address information.
    serverAddr = defineSocket(portNo); //passing port number that taken as argument.
    bindSocket(serverSocket,serverAddr); //binding socket
    listenConnections(serverSocket,MAX_CLIENT,portNo); //listening incoming connections.
    
    while (keepRunning)
    {
        //assigning new socket descriptor to client's sockID
        Client[clientCount].sockID = acceptConnection(serverSocket,&Client[clientCount],clientCount);
        Client[clientCount].index = clientCount;

        //creating thread for client
        pthread_create(&thread[clientCount], NULL, sendAndReceive, (void *)&Client[clientCount]);
        clientCount++;
    }
    signal(SIGINT,quitHandler); //closing socket and quit.

} //end main()