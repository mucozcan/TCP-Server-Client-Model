/*
A simple TCP Server.
Author: Mücteba Özcan.
References: https://beej.us/guide/bgnet/html//index.html
            https://www.ozanselte.com/c-ile-soket-programlama-rehberi/

Usage: ./Server [The port number that you want]

        in another terminal: telnet localhost [Port number that you defined.]

        If you type "exit" as Server input, it closes the connection and prints session report.
*/

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <csignal>

using namespace std;

#define ERROR -1 //value of errno
#define MAX_DATA 1024


void sessionReport(int bytesWritten, int bytesRead)//prints amount of data that transfered during session.
{
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
    cout << "Connection closed..." << endl;
}


int main(int argc, char **argv) //pass a port number as a parameter at start-up.
{
    
    int sockfd, newfd;
    //sockaddr is a structure for handling internet addresses.
    //sockaddr_in is for IPv4 AF_INET sockets.
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    char msg[MAX_DATA];
    int receivedByte, sentByte, structSize;

    //int socket(int domain, int type, int protocol), returns -1 in case of error.
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET = IPv4 Socket, SOCK_STREAM = TCP, 0 = IP Protocol

    if (sockfd == ERROR)
    {
        perror("socket"); //printing error description
    }

    //Defining server's address information.
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1])); //takes port number as an argument.
                                                //htons is used for converting native byte order
                                                //to network byte order.(little-endian to big-endian)

    serverAddr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY is used when you don't need to bind a socket to a specific IP.
                                             //When you use this value as the address when calling bind(),
                                             //the socket accepts connections to all the IPs of the machine.
    memset(&(serverAddr.sin_zero), '\0', 8); //???????

    //bind() is used for associating socket with a port on local machine. If it is not used, OS will define a port automatically.
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == ERROR) //bind(socket, address, lenght of an address).
                                                                                        //You can pass "sockaddr" as a parameter, instead of "sockaddr_in"
    {
        perror("bind");
    }
    //listen() tell a socket to listen for incoming connections.
    if (listen(sockfd, 10) == ERROR) //listen(socket, queued connections)
    {
        perror("listen");
    }

    //accept an incoming connection on a listening socket.
    int addr_size = sizeof clientAddr;
    newfd = accept(sockfd, (struct sockaddr *)&clientAddr, (socklen_t*)&addr_size); //accept() returns the newly connected socket descriptor
                                                                        //to use for subsequent communication with the newly connected client.
                                                                        //The old socket that used for listening is still there.
                                                                        //You can pass "sockaddr" as a parameter, instead of "sockaddr_in"
    if (newfd == ERROR)
    {
        perror("accept");
    }
    cout << "Connected with client!" << endl;
    int bytesRead, bytesWritten = 0; //to track the amount of sent and received data.
    while (1) //Infinite loop to keep server running.
    {
        //clear the buffer
        memset(&msg, 0, sizeof(msg));
        //recv() receive data on a socket. Returns size of received byte or -1 in case of error.
        receivedByte = recv(newfd, &msg, MAX_DATA, 0);
        if (receivedByte == ERROR)
        {
            perror("recv");
        }
        else if (receivedByte == 0) //if the remote side has closed the connection, recv() will return 0.
        {
            cout << "The client is disconnect." << endl;
        }
        cout << "Client: " << msg << endl;
        bytesRead += receivedByte;
        //Typing the respond data.
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); //clear the buffer
        strcpy(msg, data.c_str());
        if (data == "exit") //close the connection.
        {   
            sessionReport(bytesWritten,bytesRead);
            break;
        }
        //send() sends data out over a socket.
        sentByte = send(newfd, (char *)&msg, strlen(msg), 0); //send() returns the number of bytes sent, or -1 on error.
        if (sentByte == -1)
        {
            perror("send");
        }
        bytesWritten += sentByte;
    }
    close(newfd);
    close(sockfd);
    return 0;
}