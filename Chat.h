#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>

//for error handling
#include <errno.h>
#include <sys/time.h>

#define ERROR -1
#define MAX_DATA 1024
#define MAX_CLIENT 1024
#define MAX_THREAD 1024

int clientCount = 0;
FILE *errorFile;
FILE *logFile;

struct client
{
    int index;
    int sockID;
    struct sockaddr_in clientAddr;
    int len;
};

struct client Client[MAX_CLIENT];
pthread_t thread[MAX_THREAD];

//Functions
void *sendAndReceive(void *ClientDetail);
void *receiveMessages(void *sockID);
int createSocket();
struct sockaddr_in defineSocket();
void bindSocket(int serverSocket, struct sockaddr_in server);
void listenConnections(int serverSocket, int backlog, int port);
int acceptConnection(int server, struct client *client, int clientCount);
void connectToServer(int clientSocket, struct sockaddr_in server);
const char *getCurrentTime();
void logErrors(char *ErrorString, int errNo);
void logOperations(char *LogString);

const char *getCurrentTime() //returns current time and date.
{
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    return asctime(info);
}

void logErrors(char *errorString, int errNo) //writes errors to a txt file.
{
    errorFile = fopen("errorLog.txt", "ab");
    fprintf(errorFile, "%s: %s , %s\n", errorString, strerror(errNo), getCurrentTime());
    fclose(errorFile);
}
void logOperations(char *LogString)
{
    logFile = fopen("operationLog.txt", "ab");
    fprintf(logFile, "%s, %s", LogString, getCurrentTime());
    fclose(logFile);
}
void *sendAndReceive(void *ClientDetail) //thread function. Takes client struct as parameter and run send and receive functions
                                         //between clients.
{
    struct client *clientDetail = (struct client *)ClientDetail; //assigning param to a client struct.
    int index = clientDetail->index;
    int clientSocket = clientDetail->sockID;

    printf("Client %d connected.\n", index + 1);
    //logging
    char logString[] = "Connected with client ";
    sprintf(logString, "%s %d", logString, index + 1);
    logOperations(logString);

    while (1)
    {
        char data[MAX_DATA];
        int read = recv(clientSocket, data, MAX_DATA, 0); //Read command from client(SEND,LIST).
        data[read] = '\0';                                // '\0' is used for ending string.

        char output[MAX_DATA];
        //if the data is "LIST", server sends list of other clients.
        if (strcmp(data, "LIST") == 0)
        {

            for (int i = 0; i < clientCount; i++)
            {
                if (i != index)
                    snprintf(output, 1024, "Client %d is at socket %d.", i + 1, Client[i].sockID);
                logOperations(output);
            }

            send(clientSocket, output, MAX_DATA, 0);
            continue;
        }
        //if the data is "SEND", server sends message to client(ID)
        if (strcmp(data, "SEND") == 0)
        {
            read = recv(clientSocket, data, MAX_DATA, 0); //read ID of other client
            data[read] = '\0';

            int id = atoi(data) - 1;

            read = recv(clientSocket, data, MAX_DATA, 0); //read message from client
            data[read] = '\0';
            send(Client[id].sockID, data, MAX_DATA, 0); //send message to client(ID).
            sprintf(data, "%s, sent from client %d to client %d", data, index + 1, Client[id].index + 1);
            logOperations(data);
        }
    }

    return NULL;
}

void *receiveMessages(void *sockID)
{
    int clientSocket = *((int *)sockID);

    while (1)
    {
        char data[MAX_DATA];
        int read = recv(clientSocket, data, MAX_DATA, 0);
        if(read == 0)
        {   
            printf("Connection lost with server.\n");
            logOperations("Connection lost with server");
            close(clientSocket);
            exit(1);
        }
        data[read] = '\0';
        printf("%s\n", data);
    }
}

//creating a socket
int createIPv4Socket()
{
    //int socket(int domain, int type, int protocol), returns -1 in case of error.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET = IPv4 Socket, SOCK_STREAM = TCP, 0 = IP Protocol
    if (sockfd == ERROR)
    {
        perror("an error occured while creating a socket");
        logErrors("an error occured while creating a socket", errno);
    }
    printf("Created socket.\n");
    logOperations("Created socket");
    return sockfd;
}
//Defining server's address information.
struct sockaddr_in defineSocket(int port)
{
    struct sockaddr_in Server;
    Server.sin_family = AF_INET;
    Server.sin_port = htons(port); //takes port number as an argument.
                                   //htons is used for converting native byte order
                                   //to network byte order.(little-endian to big-endian)

    Server.sin_addr.s_addr = htons(INADDR_ANY); //INADDR_ANY is used when you don't need to bind a socket to a specific IP.
                                                //When you use this value as the address when calling bind(),
                                                //the socket accepts connections to all the IPs of the machine
    printf("Socket address info is defined.\n");
    logOperations("Socket address info is defined.");

    return Server;
}

void bindSocket(int serverSocket, struct sockaddr_in server)
{
    //bind() is used for associating socket with a port on local machine. If it is not used, OS will define a port automatically.
    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == ERROR)
    {
        perror("an error occured while binding");
        logErrors("an error occured while binding", errno);
        exit(1);
    }
    printf("Socket is binded.\n");
    logOperations("Socket is binded.");
}

void listenConnections(int serverSocket, int backlog, int port)
{
    //listen() tell a socket to listen for incoming connections.
    if (listen(serverSocket, MAX_CLIENT) == ERROR)
    {
        perror("an error occured while listening");
        logErrors("an error occured while listening", errno);
    }
    printf("Server started listening on port %d.\n", port);
    //logging
    char logString[] = "Server started listening on port ";
    sprintf(logString, "%s %d", logString, port);
    logOperations(logString);
}

int acceptConnection(int server, struct client *client, int clientCount)
{
    int connectionSocket = accept(server, (struct sockaddr *)&client[clientCount].clientAddr, &client[clientCount].len); //accept an incoming connection on a listening socket.
    if (connectionSocket == ERROR)
    {
        perror("an error occured while accepting");
        logErrors("an error occured while accepting", errno);
    }

    return connectionSocket;
}

void connectToServer(int clientSocket, struct sockaddr_in server)
{
    //connect() is used to connect socket that listening incoming connections.
    if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) == ERROR)
    {
        perror("an error occured while connecting to server");
        logErrors("an error occured while connecting to server", errno);
        exit(1); //log it
    }
    printf("Connection established with server.\n");
    char logString[] = "Connection established between server and client at socket";
    sprintf(logString, "%s %d", logString, clientSocket);
    logOperations(logString);
}

#endif