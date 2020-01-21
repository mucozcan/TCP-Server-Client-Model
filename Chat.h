#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
//for error handling
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
//for Base64 encoding & decoding
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#define ERROR -1
#define MAX_DATA 1024
#define MAX_CLIENT 1024
#define MAX_THREAD 1024
#define COM_PORT 5880 //constant communication port for authorized users.

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
    //int validUser;
};

struct client Client[MAX_CLIENT];
pthread_t thread[MAX_THREAD]; //thread that handles communication.



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
char *base64encode (const void *messageToEncode);
char *base64decode (const void *messageToDecode);


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
    printf("CLient count: %d\n", clientCount);
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
                if (i != index)
                    offset += snprintf(output + offset, 1024, "Client %d is at socket %d.\n", i + 1, Client[i].sockID);
                logOperations(output);
            }

            char *encodedOutput = base64encode(output);

            send(clientSocket, encodedOutput, MAX_DATA, 0);
            free(encodedOutput); //Frees up the memory holding base64 encoded data.
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
            printf("\nDATA from socket %d to socket %d: %s\n", clientSocket, Client[id].sockID, data); //printing encyrpted message.
            send(Client[id].sockID, data, MAX_DATA, 0);                                                //send message to client(ID).
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

        char *decodedMessage  = base64decode(data);
        printf("%s\n", decodedMessage);
        free(decodedMessage); //Frees up the memory holding base64 decoded data.
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
    char *decodedInfo = base64decode(userInfo);
    char user[20][128];
    printf("\nClient Username and password: %s\n", decodedInfo);

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
        if (strcmp(user[i], decodedInfo) == 0)
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
    free(decodedInfo);
    //memset(&userInfo, 0, sizeof(decodedInfo));

    return isValid;
}

char *base64encode (const void *messageToEncode){
    int lengthOfMessage = strlen(messageToEncode);
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    BUF_MEM *mem_bio_mem_ptr;    //Pointer to a "memory BIO" structure holding our base64 data.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                           //Initialize our memory sink BIO.
    BIO_push(b64_bio, mem_bio);            //Link the BIOs by creating a filter-sink BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);  //No newlines every 64 characters or less.
    BIO_write(b64_bio, messageToEncode, lengthOfMessage); //Records base64 encoded data.
    BIO_flush(b64_bio);   //Flush data.  Necessary for b64 encoding, because of pad characters.
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);  //Store address of mem_bio's memory structure.
    BIO_set_close(mem_bio, BIO_NOCLOSE);   //Permit access to mem_ptr after BIOs are destroyed.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);   //Makes space for end null.
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';  //Adds null-terminator to tail.
    return (*mem_bio_mem_ptr).data; //Returns base-64 encoded data. (See: "buf_mem_st" struct).
}

char *base64decode (const void *messageToDecode){
     int lengthOfMessage = strlen(messageToDecode);
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    char *base64_decoded = calloc( (lengthOfMessage*3)/4+1, sizeof(char) ); //+1 = null.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                         //Initialize our memory source BIO.
    BIO_write(mem_bio, messageToDecode, lengthOfMessage); //Base64 data saved in source.
    BIO_push(b64_bio, mem_bio);          //Link the BIOs by creating a filter-source BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);          //Don't require trailing newlines.
    int decoded_byte_index = 0;   //Index where the next base64_decoded byte should be written.
    while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ){ //Read byte-by-byte.
        decoded_byte_index++; //Increment the index until read of BIO decoded data is complete.
    } //Once we're done reading decoded data, BIO_read returns -1 even though there's no error.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    return base64_decoded;        //Returns base-64 decoded data with trailing null terminator.
}

#endif