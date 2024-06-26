#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <netinet/in.h>

// 代表一个战场
struct Battlefield {
    int id;
    int client1_socket;
    int client2_socket;
    bool is_full;
    std::queue<std::string> client1_messages;
    std::queue<std::string> client2_messages;
    std::mutex mtx;

    Battlefield() : id(-1), client1_socket(-1), client2_socket(-1), is_full(false) {}

    // Battlefield(const Battlefield& other) 
    //     : id(other.id), 
    //       client1_socket(other.client1_socket), 
    //       client2_socket(other.client2_socket), 
    //       is_full(other.is_full) {}

    // 拷贝赋值操作符
    Battlefield& operator=(const Battlefield& other) {
        if (this == &other) return *this;
        id = other.id;
        client1_socket = other.client1_socket;
        client2_socket = other.client2_socket;
        client1_messages = other.client1_messages;
        client2_messages = other.client2_messages;
        is_full = other.is_full;
        // mutex 不复制
        return *this;
    }
};

class Server {
public:
    Server(int port);
    ~Server();

    void start();
    void stop();

private:
    void accept_clients();
    void handle_client(int client_socket);
    void match_clients(int client_socket, int battlefield_id);
    void synchronize_game_state();
    void process_client_input(int client_socket, const std::string& input);
    void forward_messages();

    int server_socket_;
    int port_;
    std::unordered_map<int, int> client_battlefield_map_;  // client_socket -> battlefield_id
    std::unordered_map<int, Battlefield> battlefields_;    // battlefield_id -> Battlefield
    std::mutex battlefields_mtx_;
    std::vector<std::thread> threads_;
    bool running_;
};

#endif // SERVER_H