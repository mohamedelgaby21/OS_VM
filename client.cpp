#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

void receive_messages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read > 0) {
            
            cout << "\r" << buffer << "\n> " << flush;
        } else {
            cout << "\nConnection lost." << endl;
            exit(0);
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Failed to connect to server." << endl;
        return -1;
    }

    cout << "Successfully connected to the chat!" << endl;

    
    thread(receive_messages, sock).detach();

    string input;
    while (true) {
        cout << "> ";
        getline(cin, input);
        if (input == "quit") break;
        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    return 0;
}