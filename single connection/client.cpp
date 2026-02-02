#include <iostream>              // Provides input/output streams
#include <cstring>               // Provides string handling functions
#include <cstdio>                // Provides perror
#include <netinet/in.h>          // Provides network address structures
#include <arpa/inet.h>           // Provides inet_pton for IP conversion
#include <sys/socket.h>          // Provides socket system calls
#include <unistd.h>              // Provides close()
#include <thread>                // Provides threading support

using namespace std;             // Avoid std:: prefix

void receiveMessages(int sock) { // Thread function to receive messages
    char buffer[1024];           // Buffer for incoming data

    while (true) {               // Loop until server disconnects
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0); // Receive data

        if (bytesRead <= 0) {    // Detect disconnection or error
            cout << "\nServer disconnected\n"; // Notify user
            exit(0);             // Terminate program
        }

        buffer[bytesRead] = '\0'; // Null-terminate data
        cout << "\nServer: " << buffer << "\nYou: "; // Display server message
        cout.flush();             // Flush output buffer
    }
}

void sendMessages(int sock) {    // Thread function to send messages
    char buffer[1024];           // Buffer for outgoing messages

    while (true) {               // Loop to continuously send messages
        cout << "You: ";         // Prompt user
        cin.getline(buffer, sizeof(buffer)); // Read input line

        if (strcmp(buffer, "exit") == 0) { // Check exit command
            shutdown(sock, SHUT_RDWR); // Gracefully close connection
            close(sock);          // Close socket
            exit(0);              // Terminate program
        }

        send(sock, buffer, strlen(buffer), 0); // Send message to server
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (clientSocket < 0) { perror("socket"); return 1; } // Check socket creation

    sockaddr_in serverAddress{};                      // Zero-initialize address
    serverAddress.sin_family = AF_INET;               // Use IPv4
    serverAddress.sin_port = htons(8080);             // Server port number
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr); // Convert IP address

    connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)); // Connect to server
    cout << "Connected to server\n";                   // Confirm connection

    thread recvThread(receiveMessages, clientSocket);  // Start receive thread
    thread sendThread(sendMessages, clientSocket);     // Start send thread

    recvThread.join();                                 // Wait for receive thread
    sendThread.join();                                 // Wait for send thread

    close(clientSocket);                               // Close socket
    return 0;                                          // Exit program
}
