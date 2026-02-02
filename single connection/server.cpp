#include <iostream>              // Provides input/output streams (cout, cin)
#include <cstring>               // Provides C string functions (strcmp, strlen)
#include <cstdio>                // Provides perror for error printing
#include <netinet/in.h>          // Provides sockaddr_in and INET constants
#include <sys/socket.h>          // Provides socket-related system calls
#include <unistd.h>              // Provides close(), read(), write()
#include <thread>                // Provides std::thread for concurrency

using namespace std;             // Avoids std:: prefix everywhere

void receiveMessages(int sock) { // Function to continuously receive messages
    char buffer[1024];           // Buffer to store incoming data

    while (true) {               // Infinite loop to keep connection alive
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0); // Receive data from client

        if (bytesRead <= 0) {    // Check if client disconnected or error occurred
            cout << "\nClient disconnected\n"; // Notify server user
            exit(0);             // Terminate program immediately
        }

        buffer[bytesRead] = '\0'; // Null-terminate received data
        cout << "\nClient: " << buffer << "\nYou: "; // Display client message
        cout.flush();             // Force output to appear immediately
    }
}

void sendMessages(int sock) {    // Function to continuously send messages
    char buffer[1024];           // Buffer to store outgoing messages

    while (true) {               // Infinite loop for sending messages
        cout << "You: ";         // Prompt server user
        cin.getline(buffer, sizeof(buffer)); // Read full line from terminal

        if (strcmp(buffer, "exit") == 0) { // Check for exit command
            shutdown(sock, SHUT_RDWR); // Gracefully close both send and receive
            close(sock);          // Close the socket
            exit(0);              // Terminate program
        }

        send(sock, buffer, strlen(buffer), 0); // Send message to client
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (serverSocket < 0) { perror("socket"); return 1; } // Check socket creation

    int opt = 1;                                      // Enable socket option flag
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Allow port reuse

    sockaddr_in serverAddress{};                      // Zero-initialize server address
    serverAddress.sin_family = AF_INET;               // Use IPv4
    serverAddress.sin_port = htons(8080);             // Set port number
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);// Listen on all interfaces

    bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)); // Bind socket
    listen(serverSocket, 1);                           // Listen for incoming connections

    cout << "Waiting for client...\n";                 // Inform server user
    int clientSocket = accept(serverSocket, nullptr, nullptr); // Accept client connection
    cout << "Client connected\n";                      // Confirm connection

    thread recvThread(receiveMessages, clientSocket);  // Start receiving thread
    thread sendThread(sendMessages, clientSocket);     // Start sending thread

    recvThread.join();                                 // Wait for receive thread to finish
    sendThread.join();                                 // Wait for send thread to finish

    close(serverSocket);                               // Close listening socket
    return 0;                                          // Exit program
}
