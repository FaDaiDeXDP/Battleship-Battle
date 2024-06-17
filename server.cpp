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

class ChatRoom {
public:
    std::vector<int> clients; // 存储聊天室内的客户端socket
    std::string roomID;
    std::mutex mtx;

    ChatRoom() = default; // 默认构造函数

    ChatRoom(std::string id) : roomID(id) {}

    void broadcastMessage(const std::string &message, int senderSocket) {
        std::lock_guard<std::mutex> lock(mtx);
        for (int clientSocket : clients) {
            if (clientSocket != senderSocket) {
                send(clientSocket, message.c_str(), message.size(), 0);
            }
        }
    }

    void addClient(int clientSocket) {
        std::lock_guard<std::mutex> lock(mtx);
        clients.push_back(clientSocket);
    }

    void removeClient(int clientSocket) {
        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }

    bool isFull() {
        return clients.size() >= 2; // 每个聊天室最大容纳2人
    }

    bool isEmpty() {
        return clients.empty();
    }
};

class Server {
public:
    std::map<std::string, ChatRoom> chatRooms;
    std::map<int, std::string> clientToRoom;
    std::map<int, std::string> clientToID;
    std::mutex mtx;

    void handleClient(int clientSocket) {
        char buffer[1024];
        std::string clientID, roomID;

        // 接收客户端ID
        recv(clientSocket, buffer, sizeof(buffer), 0);
        clientID = buffer;

        // 接收聊天室ID
        recv(clientSocket, buffer, sizeof(buffer), 0);
        roomID = buffer;

        {
            std::lock_guard<std::mutex> lock(mtx);

            if (clientToID.find(clientSocket) != clientToID.end() || chatRooms[roomID].isFull()) {
                std::string rejectMessage = "Connection rejected.";
                send(clientSocket, rejectMessage.c_str(), rejectMessage.size(), 0);
                close(clientSocket);
                return;
            }

            if (chatRooms.find(roomID) == chatRooms.end()) {
                chatRooms.emplace(roomID, roomID);
            }

            chatRooms[roomID].addClient(clientSocket);
            clientToRoom[clientSocket] = roomID;
            clientToID[clientSocket] = clientID;
        }

        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                break;
            }

            buffer[bytesReceived] = '\0';
            std::string message = clientID + ": " + buffer;

            if (message == "end") {
                break;
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                chatRooms[roomID].broadcastMessage(message, clientSocket);
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            chatRooms[roomID].removeClient(clientSocket);
            if (chatRooms[roomID].isEmpty()) {
                chatRooms.erase(roomID);
            }

            clientToRoom.erase(clientSocket);
            clientToID.erase(clientSocket);
        }

        close(clientSocket);
    }

    void startServer(int port) {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, SOMAXCONN);

        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientSize = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

            std::thread(&Server::handleClient, this, clientSocket).detach();
        }

        close(serverSocket);
    }
};

int main() {
    Server server;
    server.startServer(54000); // 指定服务器监听的端口
    return 0;
}
