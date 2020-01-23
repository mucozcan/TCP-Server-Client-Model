/*
A server-side code of simple TCP Chat App.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/
            https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872
            http://doctrina.org/Base64-With-OpenSSL-C-API.html

Usage: ./ChatServer [The port number that you want]

        in another terminal: ./ChatClient [Port number that you defined.] [username] [password]

Client can't see directly which is the communication port.To connect to the com. port, client need to be verified from server's authentication port(given as argument to the program) first.
Server does all send and receive functions in multithread function sendAndReceive that defined in Chat.h
Clients send them encrypted messages so server can't see the content of a message.
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

static int keepRunning = 1;
static int comSocket;  //communication socket.
static int authSocket; //authentication socket.

void quitHandler(int server) //function that handles ^C.
{
    keepRunning = 0;
    close(comSocket); //closing socket.
    char logString[256];
    printf("\nSocket: %d is closed\nClosing application... Operations and errors logged.\n", comSocket);
    sprintf(logString, "Socket: %d is closed, Closing application...", comSocket);
    logOperations(logString);
    exit(1);
}

int main(int argc, char **argv)
{
    signal(SIGINT, quitHandler);
    //sockaddr_in is a structure for handling internet addresses.
    struct sockaddr_in comAddr;   //for communication socket.
    struct sockaddr_in authAddr;  //for authentication socket.
    int authPort = atoi(argv[1]); //taking authentication port as parameter.
    comSocket = createIPv4Socket();
    authSocket = createIPv4Socket();

    comAddr = defineSocket(COM_PORT); //Defining communication address information.
                                      //COM_PORT is a constant port number.

    authAddr = defineSocket(authPort); //Defining authentication address information.
    bindSocket(comSocket, comAddr);    //binding communication socket
    bindSocket(authSocket, authAddr);  //binding authentication socket.

    listenConnections(comSocket, MAX_CLIENT, COM_PORT);  //listening incoming connections on communication port.
    listenConnections(authSocket, MAX_CLIENT, authPort); //listening incoming connections on authentication port.

    while (keepRunning)
    {
        Client[clientCount].sockID = acceptConnection(authSocket, &Client[clientCount], clientCount); //first, client connects to the authentication port.
        if (checkUserInfo(Client[clientCount].sockID) == 1)                                           //checks if user info is valid.
        {
            Client[clientCount].sockID = acceptConnection(comSocket, &Client[clientCount], clientCount); //if user is valid, connects user to the communication port
            Client[clientCount].index = clientCount;

            //creating thread that handle communication for each client.
            pthread_create(&thread[clientCount], NULL, sendAndReceive, (void *)&Client[clientCount]);
            clientCount++;
            continue;
        }
    }

} //end main()