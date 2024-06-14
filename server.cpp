#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const int INTERVAL_SECONDS = 5;

void sendData(int client_socket) {
    const char* data = "Hello from server!";
    while (true) {
        send(client_socket, data, strlen(data), 0);
        std::cout << "Data sent: " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(INTERVAL_SECONDS));
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    sendData(client_socket);

    close(client_socket);
    close(server_fd);
    return 0;
}
