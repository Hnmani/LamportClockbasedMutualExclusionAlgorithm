#include "network.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <random>

class Lamport
{
    std::mutex _mutex;
    int myID;
    int clock = 0;
    std::unique_ptr<Network> n;

    void updateClock(int receiveTimeStamp)
    {
        _mutex.lock();
        clock = std::max(receiveTimeStamp, clock) + 1;
        std::cout << "Clock Updated to " << clock << std::endl;
        _mutex.unlock();
    }

public:
    Lamport(int id) : myID(id), n(std::make_unique<Network>(id)) {}

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
                std::cerr << "Should not have done that" << std::endl;
                exit(0);
            }

            std::string message(buffer);
            int receivedClock = std::stoi(message);
            updateClock(receivedClock);
        }
    }

    void sendMessage()
    {
        updateClock(clock);
        _mutex.lock();
        std::string message = std::to_string(clock) + " from " + std::to_string(myID);
        n->sendMessageToAll(message);
        _mutex.unlock();
    }

    void sleeper()
    {
        // Generate a random number of seconds between 1 and 3
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0, 3.0);
        double sleep_time = dis(gen);

        // Sleep for the random duration
        std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
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
        while (true)
        {
            std::thread slp([this]
                            { sleeper(); });
            slp.join();
            sendMessage();
        }
    }
};

int main()
{
    int id;
    std::cout << "Enter node ID: ";
    std::cin >> id;
    Lamport l(id);
    l.run();
    return 0;
}
