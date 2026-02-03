/**
 * Task 1: Iterative Server - Chat Application
 * 
 * This server handles ONE client connection at a time (iterative/sequential).
 * It creates a TCP socket, binds to a port, listens for connections,
 * and enables bidirectional chat with the connected client.
 * 
 * Compile (Windows): g++ -o server.exe server.cpp -lws2_32
 * Compile (Linux):   g++ -o server server.cpp -pthread
 * 
 * Course: 23CSE312 - Distributed Systems
 * Lab: Socket Programming (Chat Application)
 */

#ifdef _WIN32
    // Windows-specific headers for socket programming
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    // Linux/Unix-specific headers for socket programming
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>

using namespace std;

// Global flag to signal threads to stop
atomic<bool> running(true);

/**
 * Function: receiveMessages
 * Purpose: Continuously receives messages from the connected client
 * Parameters: sock - The socket descriptor for communication
 * 
 * This function runs in a separate thread and displays incoming
 * messages from the client. It terminates when the client disconnects.
 */
void receiveMessages(SOCKET sock) {
    char buffer[1024];  // Buffer to store incoming messages
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));  // Clear buffer before receiving
        
        // Receive data from client (blocking call)
        int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            // Client disconnected or error occurred
            cout << "\n[Server] Client disconnected." << endl;
            running = false;
            break;
        }
        
        buffer[bytesRead] = '\0';  // Null-terminate the received string
        cout << "\n[Client]: " << buffer << endl;
        cout << "[You]: " << flush;  // Prompt for next message
    }
}

/**
 * Function: sendMessages
 * Purpose: Continuously sends messages to the connected client
 * Parameters: sock - The socket descriptor for communication
 * 
 * This function runs in a separate thread and reads user input
 * to send messages to the client. Type "exit" to disconnect.
 */
void sendMessages(SOCKET sock) {
    char buffer[1024];  // Buffer to store outgoing messages
    
    while (running) {
        cout << "[You]: " << flush;
        cin.getline(buffer, sizeof(buffer));  // Read user input
        
        if (!running) break;  // Check if other thread signaled to stop
        
        // Check for exit command
        if (strcmp(buffer, "exit") == 0) {
            cout << "[Server] Closing connection..." << endl;
            running = false;
            break;
        }
        
        // Send message to client
        send(sock, buffer, strlen(buffer), 0);
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "  Task 1: Iterative Server (TCP Chat)  " << endl;
    cout << "========================================" << endl;
    
#ifdef _WIN32
    // Initialize Winsock (Windows only)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[Error] WSAStartup failed!" << endl;
        return 1;
    }
    cout << "[Server] Winsock initialized successfully." << endl;
#endif

    // Step 1: Create a TCP socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "[Error] Failed to create socket!" << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "[Server] Socket created successfully." << endl;
    
    // Step 2: Set socket options (allow address reuse)
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    // Step 3: Configure server address structure
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;         // IPv4
    serverAddress.sin_port = htons(8080);       // Port 8080
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    
    // Step 4: Bind socket to the address
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "[Error] Failed to bind socket!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "[Server] Socket bound to port 8080." << endl;
    
    // Step 5: Listen for incoming connections (backlog = 1 for iterative server)
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        cerr << "[Error] Failed to listen on socket!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "[Server] Listening for connections..." << endl;
    cout << "[Server] Waiting for a client to connect..." << endl;
    
    // Step 6: Accept a client connection (blocking call)
    sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientLen);
    
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[Error] Failed to accept connection!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Get client IP address for display
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
    cout << "[Server] Client connected from " << clientIP << endl;
    cout << "[Server] Chat session started. Type 'exit' to disconnect." << endl;
    cout << "----------------------------------------" << endl;
    
    // Step 7: Create threads for sending and receiving messages
    thread recvThread(receiveMessages, clientSocket);
    thread sendThread(sendMessages, clientSocket);
    
    // Wait for both threads to complete
    recvThread.join();
    sendThread.join();
    
    // Step 8: Clean up - close sockets
    cout << "[Server] Shutting down..." << endl;
    closesocket(clientSocket);
    closesocket(serverSocket);
    
#ifdef _WIN32
    WSACleanup();  // Clean up Winsock
#endif
    
    cout << "[Server] Goodbye!" << endl;
    return 0;
}
