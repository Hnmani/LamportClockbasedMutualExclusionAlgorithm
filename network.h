#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <unordered_map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Network
{
    std::string myIP;
    int myID;
    int myserverport;
    bool serverSetup = false;
    bool clientSetup = false;
    std::vector<int> senderSockets = {-1, -1, -1};
    std::vector<int> receiverSockets = {-1, -1, -1};

    std::vector<std::pair<std::string, int>> serverList = {
        {"127.0.0.1", 5103}, {"127.0.0.1", 5104}, {"127.0.0.1", 5105}};

    void setMyID(int id)
    {
        myID = id;
        myIP = serverList[id].first;
        myserverport = serverList[id].second;
    };

    void getIPandPORT(std::string &IP, int &port, struct sockaddr_in address)
    {
        char client_ip[INET_ADDRSTRLEN];
        struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&address;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        inet_ntop(AF_INET, &ipAddr, client_ip, INET_ADDRSTRLEN);
        port = ntohs(pV4Addr->sin_port);
        std::string s(client_ip);
        IP = s;
    }

    void server()
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
        {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        sockaddr_in serverAddress, clientAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(myserverport);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
        {
            std::cerr << "Error binding socket" << std::endl;
            return;
        }

        if (listen(serverSocket, 3) == -1)
        {
            std::cerr << "Error listening on socket" << std::endl;
            return;
        }
        int connections = 2;
        while (connections)
        {
            socklen_t clientAddressSize = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
            if (clientSocket == -1)
            {
                std::cerr << "Error accepting client connection" << std::endl;
                continue;
            }

            char nameBuffer[256];
            memset(nameBuffer, 0, sizeof(nameBuffer));
            int nameReceived = recv(clientSocket, nameBuffer, sizeof(nameBuffer), 0);

            if (nameReceived <= 0)
            {
                std::cerr << "Error receiving client name." << std::endl;
                close(clientSocket);
                continue;
            }
            std::string clientID(nameBuffer);
            int id = stoi(clientID);

            if (id != -1)
            {
                std::cout << "Connected to P: " << id << std::endl;
                receiverSockets[id] = clientSocket;
                connections--;
            }
        }
        serverSetup = true;
    }

    int clientConnect(std::string server_ip, int SERVER_PORT)
    {

        char *SERVER_IP = new char[INET_ADDRSTRLEN];
        strcpy(SERVER_IP, server_ip.c_str());

        int sock = 0, valread;
        struct sockaddr_in serv_addr, client_addr;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }
        while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
        };

        std::string hello = std::to_string(myID);
        send(sock, hello.c_str(), hello.size(), 0);
        return sock;
    }

    void receiver()
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == myID)
                continue;
            else
            {
                senderSockets[i] = clientConnect(serverList[i].first, serverList[i].second);
            }
        }
        clientSetup = true;
    }

    void setup()
    {
        std::thread serverThread([this]
                                 { server(); });
        serverThread.detach();
        std::thread receiverThread([this]
                                   { receiver(); });
        receiverThread.detach();
    }

public:
    Network(int id)
    {
        setMyID(id);
        setup();
        while (!serverSetup || !clientSetup)
            ;
        std::cout << "Network Setup Done" << std::endl;
    }

    ~Network()
    {
        for (int i = 0; i < 3; i++)
        {
            if (receiverSockets[i] != -1)
                close(receiverSockets[i]);
        }
        for (int i = 0; i < 3; i++)
        {
            if (senderSockets[i] != -1)
                close(senderSockets[i]);
        }
    }

    void sendMessageToAll(std::string msg)
    {
        for (int i = 0; i < 3; i++)
        {
            if (i != myID)
            {
                send(senderSockets[i], msg.c_str(), msg.size(), 0);
            }
        }
    }

    void sendMessage(int id, std::string msg)
    {
        send(senderSockets[id], msg.c_str(), msg.size(), 0);
    }

    std::vector<int> getReceiverSockets()
    {
        return receiverSockets;
    }
};
