#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

void receiveMessages(int clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Message: " << buffer << std::endl;
        } else {
            break;
        }
    }
}

void sendMessages(int clientSocket){
    std::string message = "";
    while (true) {
        std::getline(std::cin, message);
        if (message == "end") {
            send(clientSocket, message.c_str(), message.size(), 0);
            break;
        }
        send(clientSocket, message.c_str(), message.size(), 0);
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    std::string clientID;
    std::string roomID;

    std::cout << "Enter your client ID: ";
    std::cin >> clientID;
    send(clientSocket, clientID.c_str(), clientID.size(), 0);

    std::cout << "Enter the room ID you want to join: ";
    std::cin >> roomID;
    send(clientSocket, roomID.c_str(), roomID.size(), 0);

    std::thread receiveThread(receiveMessages, clientSocket);
    std::thread sendThread(sendMessages,clientSocket);

    sendThread.join();
    receiveThread.detach();

    close(clientSocket);
    return 0;
}
