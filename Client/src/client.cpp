#include <iostream>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

// 发送随机浮点数给服务器
void send_random_floats(int socket) {
    std::srand(std::time(0));
    while (true) {
        for (int i = 0; i < 5; ++i) {
            float random_float = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            std::string message = std::to_string(random_float) + "\n";
            send(socket, message.c_str(), message.size(), 0);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// 接收并打印来自服务器的消息
void receive_messages(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            std::cout << "Received from server: " << buffer << std::endl;
        } else if (bytes_received <= 0) {
            std::cerr << "Server disconnected or error occurred." << std::endl;
            close(socket);
            return;
        }
    }
}

int main() {
    const char* server_ip = "127.0.0.1";  // 服务器IP地址
    int server_port = 54000;  // 服务器端口

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to connect to server." << std::endl;
        close(client_socket);
        return 1;
    }

    std::string battlefield_id;
    std::cout << "Enter battlefield ID: ";
    std::cin >> battlefield_id;
    send(client_socket, battlefield_id.c_str(), battlefield_id.size(), 0);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        std::string response(buffer, bytes_received);
        if (response == "Battlefield full. Connection refused.\n") {
            std::cerr << "Server response: " << response << std::endl;
            close(client_socket);
            return 1;
        } else if (response == "Waiting for opponent...\n") {
            std::cout << "Server response: " << response << std::endl;
            // 等待匹配到对手
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received > 0) {
                    response = std::string(buffer, bytes_received);
                    if (response == "Matched! Start playing.\n") {
                        std::cout << "Server response: " << response << std::endl;
                        break;
                    }
                } else if (bytes_received <= 0) {
                    std::cerr << "Server disconnected or error occurred." << std::endl;
                    close(client_socket);
                    return 1;
                }
            }
        }
    } else {
        std::cerr << "Failed to receive initial response from server." << std::endl;
        close(client_socket);
        return 1;
    }

    std::thread sender_thread(send_random_floats, client_socket);
    std::thread receiver_thread(receive_messages, client_socket);

    sender_thread.join();
    receiver_thread.join();

    close(client_socket);
    return 0;
}