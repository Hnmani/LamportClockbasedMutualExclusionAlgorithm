# Lamport Mutual Exclusion Algorithm  


To run this code, we need three users.  

First modify line number 25 in **"network.h"** file to add the IP address and port  
number of the three users. In case you want to test it on a single device, you  
can use loopback address. Example:  

```
std::vector<std::pair<std::string, int>> serverList = {{"127.0.0.1", 5100}, {"127.0.0.2", 5101}, {"127.0.0.3", 5102}};
```


Next we need to complile the code using the command:  
```_
g++ lamport.cpp -o lamport
```

Finally run the executable for three different users:  
```
./lamport
```

In case you are testing on loopback address, you can use three different terminals.  

It will ask the user to "Enter node ID:".  
The users need to enter the index of their {IP address, port number} pair.  
According to above example:

```
User with {"127.0.0.1", 5100} will enter 0.  
User with {"127.0.0.2", 5101} will enter 1.  
User with {"127.0.0.3", 5102} will enter 2.  
```

Each process will print when they enter and leave the critical section.

  
**Name and Roll Numbers of the students:  
Suraj Anand: 23CS06005  
Archit Kumar: 23CS06016  
Harsh Neel Mani: 23CS06021**  
