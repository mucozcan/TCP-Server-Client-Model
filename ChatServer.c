/*
A server-side code of simple TCP Chat App.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/
            https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872

Usage: ./ChatServer [The port number that you want]

        in another terminal: ./ChatClient [Port number that you defined.] [username] [password]
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
static int comSocket; //communication socket.
static int authSocket; //authentication socket.

void quitHandler(int server)
{
    keepRunning = 0;
    close(comSocket); //closing socket.
    char logString[256];
    printf("\nSocket: %d is closed\nClosing application... Operations and errors logged.\n",comSocket);
    sprintf(logString,"Socket: %d is closed, Closing application...",comSocket);
    logOperations(logString);
    exit(1);
}

int main(int argc, char **argv)
{
    signal(SIGINT,quitHandler);
    //sockaddr_in is a structure for handling internet addresses.
    struct sockaddr_in comAddr; //for communication socket.
    struct sockaddr_in authAddr; //for authentication socket.
    int authPort = atoi(argv[1]); //taking authentication port as parameter.
    comSocket = createIPv4Socket();
    authSocket = createIPv4Socket();

    //Defining communication address information.
    comAddr = defineSocket(COM_PORT); //COM_PORT is a constant port number.
    //Defining authentication address information.
    authAddr = defineSocket(authPort); 
    bindSocket(comSocket,comAddr); //binding communication socket
    bindSocket(authSocket,authAddr);//binding authentication socket.

    listenConnections(comSocket,MAX_CLIENT,COM_PORT); //listening incoming connections.
    listenConnections(authSocket,MAX_CLIENT,authPort); //listening incoming connections.
    
    while (keepRunning)
    {
        Client[clientCount].sockID = acceptConnection(authSocket,&Client[clientCount],clientCount); //first, client connects to the auth. port.
        if(checkUserInfo(Client[clientCount].sockID) == 1) //checks if user info is valid.
        {
            Client[clientCount].sockID = acceptConnection(comSocket,&Client[clientCount],clientCount); //if user is valid, connects user to the com. port
            Client[clientCount].index = clientCount;

            //creating thread for client
            pthread_create(&thread[clientCount], NULL, sendAndReceive, (void *)&Client[clientCount]);
            clientCount++;
            continue;
        }
        
        
  
    
    }

} //end main()