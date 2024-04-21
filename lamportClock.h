#include "network.h"
#include <memory>
#include <mutex>
#include <chrono>
#include <random>
#include <queue>

class Lamport
{
    std::mutex _mutex;
    int myID;
    int clock = 0;
    std::unique_ptr<Network> n;
    std::vector<bool> receivedReplies = {true, true, true};
    int receivedRepliesCount = 0;

    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> requestQueue;

    bool enteredCriticalSection = false;

    void updateClock(int receiveTimeStamp)
    {
        clock = std::max(receiveTimeStamp, clock) + 1;
    }

    void sendMessage(int receipientID, std::string message)
    {
        updateClock(clock);
        message = message + " " + std::to_string(clock);
        n->sendMessage(receipientID, message);
    }

    void sendMessage(std::string message)
    {
        updateClock(clock);
        message = message + " " + std::to_string(clock);
        n->sendMessageToAll(message);
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

            _mutex.lock();
            std::string message(buffer);
            int pos = message.find(' ');
            int receivedClock = std::stoi(message.substr(pos + 1));
            message = message.substr(0, pos);
            updateClock(receivedClock);
            if (message == "REQUEST")
            {
                sendMessage(id, "REPLY");
                requestQueue.push(std::make_pair(receivedClock, id));
            }

            if (message == "REPLY")
            {
                serviceReply(receivedClock, id);
            }

            if (message == "RELEASE")
            {
                serviceRelease(receivedClock, id);
            }
            _mutex.unlock();
        }
    }

    void sleeper()
    {
        srand(time(0));
        int sleep_time = rand() % 7;
        // Sleep for the random duration
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
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
            enterCriticalSection();
        }
    }

    int broadcastRequest()
    {
        sendMessage("REQUEST");
        int currClock = clock;
        return currClock;
    };

    void serviceReply(int receivedClock, int senderID)
    {
        if (receivedReplies[senderID] == false)
        {
            receivedReplies[senderID] = true;
            receivedRepliesCount++;
        }
    };

    void serviceRelease(int receivedClock, int senderID)
    {
        requestQueue.pop();
    };

    void releaseCriticalSection()
    {
        requestQueue.pop();
        sendMessage("RELEASE");
    };

    void enterCriticalSection()
    {
        _mutex.lock();
        int currClock = broadcastRequest();
        std::pair<int, int> p = std::make_pair(currClock, myID);
        requestQueue.push(p);
        for (int i = 0; i < 3; i++)
        {
            if (i != myID)
                receivedReplies[i] = false;
        }
        receivedRepliesCount = 0;
        _mutex.unlock();

        _mutex.lock();
        while (receivedRepliesCount != 2 || requestQueue.top() != p)
        {
            _mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            _mutex.lock();
        }
        _mutex.unlock();

        std::cout << "Entered Critical Section" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Leaving Critical Section" << std::endl;

        _mutex.lock();
        releaseCriticalSection();
        _mutex.unlock();
    }
};
