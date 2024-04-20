#include <iostream>
#include <thread>

class LamportClock
{
    int time;

public:
    void updateClock();
    void updateClock(int recievedTime);
    int getTimeStamp();
};
int main()
{
}