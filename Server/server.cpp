#include "server.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>

// 构造函数，初始化服务器
Server::Server(int port) : port_(port), running_(false) {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_, 10) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        exit(EXIT_FAILURE);
    }
}

// 析构函数，停止服务器并清理资源
Server::~Server() {
    stop();
}

// 启动服务器
void Server::start() {
    running_ = true;
    threads_.emplace_back(&Server::accept_clients, this);
    threads_.emplace_back(&Server::forward_messages, this);

    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// 停止服务器
void Server::stop() {
    running_ = false;
    close(server_socket_);
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// 接受客户端连接
void Server::accept_clients() {
    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket == -1) {
            std::cerr << "Failed to accept client." << std::endl;
            continue;
        }

        // 将客户端socket设为非阻塞模式
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

        threads_.emplace_back(&Server::handle_client, this, client_socket);
    }
}

// 处理客户端连接
void Server::handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
            std::string input(buffer, bytes_received);
            process_client_input(client_socket, input);
        } else if (bytes_received == 0) {
            std::cerr << "Client disconnected." << std::endl;
            close(client_socket);
            return;
        } else if (bytes_received < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
            // 非阻塞模式下没有数据可读，这不是错误
            std::this_thread::yield();
        } else {
            std::cerr << "Error receiving data from client." << std::endl;
            close(client_socket);
            return;
        }
    }
}

// 匹配客户端进入战场
void Server::match_clients(int client_socket, int battlefield_id) {
    std::lock_guard<std::mutex> lock(battlefields_mtx_);

    auto it = battlefields_.find(battlefield_id);
    if (it == battlefields_.end()) {
        // 战场不存在，创建新的战场
        Battlefield bf;
        bf.id = battlefield_id;
        bf.client1_socket = client_socket;
        battlefields_[battlefield_id] = bf;
        client_battlefield_map_[client_socket] = battlefield_id;
        send(client_socket, "Waiting for opponent...\n", 24, 0);
    } else {
        Battlefield& bf = it->second;
        if (!bf.is_full) {
            // 战场有一个玩家，进行匹配
            bf.client2_socket = client_socket;
            bf.is_full = true;
            client_battlefield_map_[client_socket] = battlefield_id;
            send(bf.client1_socket, "Matched! Start playing.\n", 24, 0);
            send(bf.client2_socket, "Matched! Start playing.\n", 24, 0);
        } else {
            // 战场满员，拒绝连接
            send(client_socket, "Battlefield full. Connection refused.\n", 37, 0);
            close(client_socket);
        }
    }
}

// 同步游戏状态（每秒转发消息）
void Server::synchronize_game_state() {
    while (running_) {
        std::lock_guard<std::mutex> lock(battlefields_mtx_);
        for (auto& pair : battlefields_) {
            Battlefield& bf = pair.second;
            if (bf.is_full) {
                // 转发client1的消息给client2
                while (!bf.client1_messages.empty()) {
                    std::string message = bf.client1_messages.front();
                    bf.client1_messages.pop();
                    send(bf.client2_socket, message.c_str(), message.size(), 0);
                }
                // 转发client2的消息给client1
                while (!bf.client2_messages.empty()) {
                    std::string message = bf.client2_messages.front();
                    bf.client2_messages.pop();
                    send(bf.client1_socket, message.c_str(), message.size(), 0);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 每秒5次
    }
}

// 处理客户端输入
void Server::process_client_input(int client_socket, const std::string& input) {
    int battlefield_id = client_battlefield_map_[client_socket];
    Battlefield& bf = battlefields_[battlefield_id];
    std::lock_guard<std::mutex> lock(bf.mtx);

    if (client_socket == bf.client1_socket) {
        bf.client1_messages.push(input);
    } else if (client_socket == bf.client2_socket) {
        bf.client2_messages.push(input);
    }
}