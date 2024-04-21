#include "lamportClock.h"

int main()
{
    int id;
    std::cout << "Enter node ID: ";
    std::cin >> id;
    Lamport l(id);
    l.run();
    return 0;
}
