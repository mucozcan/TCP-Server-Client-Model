#ifndef CHAT_H
#define CHAT_H


#define ERROR -1
#define MAX_DATA 1024
#define MAX_CLIENT 1024
#define MAX_THREAD 1024


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
int createSocket();
struct sockaddr_in defineSocket();
void bindSocket(int serverSocket,struct sockaddr_in server);
void listenConnections(int serverSocket,int backlog);
int acceptConnection(int server,struct client* client,int clientCount);

void *sendAndReceive(void *ClientDetail) //thread function. Takes client struct as parameter and run send and receive functions
                                         //between clients.
{
    struct client *clientDetail = (struct client *)ClientDetail; //assigning param to a client struct.
    int index = clientDetail->index;
    int clientSocket = clientDetail->sockID;

    printf("Client %d connected.\n", index + 1);

    while (1)
    {
        char data[MAX_DATA];
        int read = recv(clientSocket, data, 1024, 0); //Read command from client(SEND,LIST).
        data[read] = '\0';                            // '\0' is used for ending string.

        char output[MAX_DATA];
        //if the data is "LIST", server sends list of other clients.
        if (strcmp(data, "LIST") == 0)
        {

            for (int i = 0; i < clientCount; i++)
            {
                if (i != index)
                    snprintf(output, 1024, "Client %d is at socket %d.\n", i + 1, Client[i].sockID);
            }

            send(clientSocket, output, 1024, 0);
            continue;
        }
        //if the data is "SEND", server sends message to client(ID)
        if (strcmp(data, "SEND") == 0)
        {
            read = recv(clientSocket, data, 1024, 0); //read ID of other client
            data[read] = '\0';

            int id = atoi(data) - 1;

            read = recv(clientSocket, data, 1024, 0); //read message from client
            data[read] = '\0';
            send(Client[id].sockID, data, 1024, 0); //send message to client(ID).
        }
    }

    return NULL;
}

//creating a socket
int createIPv4Socket()
{
    //int socket(int domain, int type, int protocol), returns -1 in case of error.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET = IPv4 Socket, SOCK_STREAM = TCP, 0 = IP Protocol
    if (sockfd == ERROR)
    {
        perror("an error occured while creating socket:");
    }
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
    return Server;
}

void bindSocket(int serverSocket, struct sockaddr_in server)
{
    //bind() is used for associating socket with a port on local machine. If it is not used, OS will define a port automatically.
    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == ERROR)
    {
        perror("an error occured while binding:"); //log it
    }
}

void listenConnections(int serverSocket,int backlog){
     //listen() tell a socket to listen for incoming connections.
     printf("listening...");
    if (listen(serverSocket, 1024) == ERROR)
    {
        perror("an error occured while listening:");
         //log it
    }
}

int acceptConnection(int server,struct client* client,int clientCount)
{
    int connectionSocket = accept(server, (struct sockaddr *)&client[clientCount].clientAddr, &client[clientCount].len); //accept an incoming connection on a listening socket.
    if(connectionSocket == ERROR)
    {
        perror("an error occured while accepting:");
    }
    return connectionSocket;
}
#endif