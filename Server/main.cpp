#include "server.h"
#include <iostream>

int main() {
    int port = 54000;  // 服务器端口

    try {
        Server server(port);
        std::cout << "Server started on port " << port << std::endl;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}