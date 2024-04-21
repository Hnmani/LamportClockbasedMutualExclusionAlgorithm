#include "network.h"
#include <mutex>

class Lamport
{
    std::mutex _mutex;
    int myID;
    int clock = 0;
    Network *n = nullptr;
    void updateClock(int receiveTimeStamp)
    {
        _mutex.lock();
        clock = std::max(receiveTimeStamp, clock + 1);
        std::cout << "Clock Updated to " << clock << std::endl;
        _mutex.unlock();
    }

public:
    Lamport(int id)
    {
        myID = id;
        n = new Network(id);
    }

    void handleClient(int clientSocket, int id)
    {
        char buffer[1024];
        while (true)
        {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0)
            {
                std::cerr << id << " disconnected." << std::endl;
                close(clientSocket);
                exit(0);
            }

            std::string message(buffer);
            std::cout << id << " : " << message << std::endl;
            int clock = stoi(message);
            updateClock(clock);
        }
    }

    void run()
    {
        std::vector<int> sockList = n->getReceiverSockets();
        std::vector<std::thread> threads;

        for (int i = 0; i < 3; i++)
        {
            if (i != myID)
            {
                threads.emplace_back(std::thread(&Lamport::handleClient, this, sockList[i], i));
            }
        }

        for (auto &t : threads)
        {
            t.detach();
        }
        std::string clocktime = std::to_string(clock);
        n->sendMessageToAll(clocktime);
    }
};

int main()
{
    int id;
    std::cin >> id;
    Lamport l = Lamport(id);
    l.run();
}