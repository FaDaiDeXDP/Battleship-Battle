#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class server{
public:
    server();
    ~server();
    void close();
    bool init(const char* ip, unsigned short port);
    
private:

};