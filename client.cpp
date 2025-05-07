#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/select.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // Main loop
    while (true) {
        std::cout << ">> ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "Q" || input == "q") break;

        // Send command
        send(sock, input.c_str(), input.length(), 0);

        // Response handling with select()
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sock, &read_set);
        
        struct timeval timeout{.tv_sec = 1, .tv_usec = 0};  // 1-second timeout
        
        while (true) {
            int ready = select(sock + 1, &read_set, NULL, NULL, &timeout);
            
            if (ready == -1) {
                perror("select error");
                break;
            }
            if (ready == 0) {  // Timeout
                std::cout << "\n[Response complete]\n";
                break;
            }

            char chunk[4096];
            ssize_t bytes_received = recv(sock, chunk, sizeof(chunk), 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) std::cout << "\n[Server disconnected]\n";
                else perror("recv error");
                close(sock);
                return -1;
            }
            
            std::cout.write(chunk, bytes_received);
        }
    }

    close(sock);
    return 0;
}
