#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

vector<int> clients;
mutex clients_mutex; 

void broadcast(string message, int sender_fd) {
    lock_guard<mutex> lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            cout << "A client has disconnected." << endl;
            lock_guard<mutex> lock(clients_mutex);
            clients.erase(remove(clients.begin(), clients.end(), client_fd), clients.end());
            close(client_fd);
            break;
        }
        
        cout << "Message received: " << buffer << endl;
        broadcast(string(buffer), client_fd);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    cout << "Server is running on port " << PORT << ". Waiting for connections..." << endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        {
            lock_guard<mutex> lock(clients_mutex);
            clients.push_back(client_fd);
        }
        cout << "New client joined the chat!" << endl;
        thread(handle_client, client_fd).detach();
    }

    return 0;
}