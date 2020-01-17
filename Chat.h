#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>

//for error handling
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>

#define ERROR -1
#define MAX_DATA 1024
#define MAX_CLIENT 1024
#define MAX_THREAD 1024
#define COM_PORT  5880 //constant communication port for authorized users.

int clientCount = 0;

FILE *errorFile;
FILE *logFile;
FILE *userDatabase;

struct client
{
    int index;
    int sockID;
    struct sockaddr_in clientAddr;
    int len;
    int validUser;
};

struct client Client[MAX_CLIENT];
pthread_t thread[MAX_THREAD]; //thread that handles communication.
//pthread_t authThread[MAX_THREAD]; //thread that handles authentication.

//Functions
void *sendAndReceive(void *ClientDetail);
void *receiveMessages(void *sockID);
int createSocket();
struct sockaddr_in defineSocket(int port);
void bindSocket(int serverSocket, struct sockaddr_in server);
void listenConnections(int serverSocket, int backlog, int port);
int acceptConnection(int server, struct client *client, int clientCount);
void connectToServer(int clientSocket, struct sockaddr_in server);
const char *getCurrentTime();
void logErrors(char *ErrorString, int errNo);
void logOperations(char *LogString);
int checkUserInfo(int ClientSocket);

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

    //assigning new socket descriptor to client's sockID

    printf("Client %d connected.\n", index + 1);
    printf("CLient count: %d\n",clientCount);
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
            int offset = 0;
            for (int i = 0; i < clientCount; i++)
            {   
                printf("clientCount:infor %d",clientCount);
                if (i != index)
                    offset += snprintf(output + offset, 1024, "Client %d is at socket %d.\n", i + 1, Client[i].sockID);
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
        if (read == 0)
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

int checkUserInfo(int ClientSocket) //checks if user is defined in database. Returns 1 if user is valid, 0 otherwise.
{
    int isValid = 0;
    int clientSocket = ClientSocket;
    char userInfo[MAX_DATA];
    recv(clientSocket, userInfo, MAX_DATA, 0); //receive user info(ID,password) from client.

    char user[20][128];
    printf("\nClient Username and password: %s\n", userInfo);

    if ((userDatabase = fopen("login.txt", "r")) == NULL) //opens database.
    {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        exit(1);
    }

    int i = 0; //counter
    int totalUser = 0;
    int found = 0;
    while (fgets(user[i], 128, userDatabase)) //scans text file and parses users to an array.
    {
        user[i][strlen(user[i]) - 1] = '\0';
        i++;
    }
    totalUser = i;
    for (i = 0; i < totalUser; ++i) //to compare array elements with received user info.
    {
        if (strcmp(user[i], userInfo) == 0)
        {
            isValid = 1;
            found = 1;
            char info[5]; //port  number
            sprintf(info, "%d", COM_PORT);
            printf("\nPort info sent to %d\n", clientCount);
            send(clientSocket, info, 1024, 0);
            break;
        }
    }
    if (found == 0) //in case of no matches.
    {

        printf("\nInvalid username or password\n");
        char info[] = "\nInvalid username or password.\n";
        send(clientSocket, info, MAX_DATA, 0);
        close(clientSocket);
    }
    fclose(userDatabase);
    memset(&userInfo, 0, sizeof(userInfo));

    return isValid;
}

#endif