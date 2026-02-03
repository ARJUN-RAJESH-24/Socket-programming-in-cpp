/**
 * Task 2: Concurrent Server - Multi-Client Chat Application
 * 
 * This server handles MULTIPLE client connections simultaneously using threads.
 * Each client gets its own thread for communication, allowing concurrent access.
 * Messages from any client are broadcast to all other connected clients.
 * 
 * Compile (Windows): g++ -o server.exe server.cpp -lws2_32
 * Compile (Linux):   g++ -o server server.cpp -pthread
 * 
 * Course: 23CSE312 - Distributed Systems
 * Lab: Socket Programming (Concurrent Chat Application)
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
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>

using namespace std;

// Configuration
const int PORT = 8080;
const int MAX_CLIENTS = 10;

// Thread-safe client list management
vector<SOCKET> clientSockets;      // List of connected client sockets
mutex clientMutex;                  // Mutex for thread-safe access to client list
atomic<bool> serverRunning(true);   // Flag to control server shutdown
atomic<int> clientCount(0);         // Number of connected clients

/**
 * Function: broadcastMessage
 * Purpose: Sends a message to all connected clients except the sender
 * Parameters: 
 *   - message: The message to broadcast
 *   - senderSocket: Socket of the sender (excluded from broadcast)
 * 
 * This function is thread-safe and broadcasts messages to all clients.
 */
void broadcastMessage(const string& message, SOCKET senderSocket) {
    lock_guard<mutex> lock(clientMutex);  // Acquire lock for thread safety
    
    for (SOCKET sock : clientSockets) {
        if (sock != senderSocket) {  // Don't send to the original sender
            send(sock, message.c_str(), message.length(), 0);
        }
    }
}

/**
 * Function: removeClient
 * Purpose: Removes a client from the list of connected clients
 * Parameters: sock - Socket of the client to remove
 * 
 * This function is thread-safe and updates the client list.
 */
void removeClient(SOCKET sock) {
    lock_guard<mutex> lock(clientMutex);  // Acquire lock for thread safety
    
    auto it = find(clientSockets.begin(), clientSockets.end(), sock);
    if (it != clientSockets.end()) {
        clientSockets.erase(it);
        clientCount--;
    }
    closesocket(sock);
}

/**
 * Function: handleClient
 * Purpose: Handles communication with a single client (runs in separate thread)
 * Parameters: 
 *   - clientSocket: Socket for this client
 *   - clientId: Unique identifier for this client
 *   - clientIP: IP address of the client
 * 
 * This function runs in its own thread, handling all messages from one client.
 * It receives messages and broadcasts them to all other connected clients.
 */
void handleClient(SOCKET clientSocket, int clientId, string clientIP) {
    char buffer[1024];
    string welcomeMsg = "[Server] Client " + to_string(clientId) + " (" + clientIP + ") joined the chat!";
    
    // Notify all clients about the new connection
    broadcastMessage(welcomeMsg, clientSocket);
    cout << welcomeMsg << endl;
    
    // Send welcome message to the new client
    string personalWelcome = "[Server] Welcome! You are Client " + to_string(clientId) + 
                             ". There are " + to_string(clientCount.load()) + " clients connected.";
    send(clientSocket, personalWelcome.c_str(), personalWelcome.length(), 0);
    
    // Main message handling loop for this client
    while (serverRunning) {
        memset(buffer, 0, sizeof(buffer));  // Clear buffer
        
        // Receive message from client
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            // Client disconnected
            string disconnectMsg = "[Server] Client " + to_string(clientId) + " (" + clientIP + ") left the chat.";
            cout << disconnectMsg << endl;
            broadcastMessage(disconnectMsg, clientSocket);
            break;
        }
        
        buffer[bytesRead] = '\0';  // Null-terminate
        
        // Format message with client identifier
        string message = "[Client " + to_string(clientId) + "]: " + string(buffer);
        cout << message << endl;  // Display on server console
        
        // Broadcast message to all other clients
        broadcastMessage(message, clientSocket);
    }
    
    // Clean up - remove client from list
    removeClient(clientSocket);
}

/**
 * Function: serverConsole
 * Purpose: Handles server-side input for broadcasting messages to all clients
 * 
 * This function runs in a separate thread and allows the server operator
 * to send messages to all connected clients. Type "quit" to shutdown server.
 */
void serverConsole() {
    char buffer[1024];
    
    cout << "[Server] Server console ready. Type messages to broadcast, or 'quit' to shutdown." << endl;
    
    while (serverRunning) {
        cin.getline(buffer, sizeof(buffer));
        
        if (strcmp(buffer, "quit") == 0) {
            cout << "[Server] Shutting down server..." << endl;
            serverRunning = false;
            
            // Notify all clients about server shutdown
            string shutdownMsg = "[Server] Server is shutting down. Goodbye!";
            lock_guard<mutex> lock(clientMutex);
            for (SOCKET sock : clientSockets) {
                send(sock, shutdownMsg.c_str(), shutdownMsg.length(), 0);
                closesocket(sock);
            }
            clientSockets.clear();
            break;
        }
        
        if (strlen(buffer) > 0) {
            string serverMsg = "[Server]: " + string(buffer);
            cout << serverMsg << endl;
            
            // Broadcast to all clients
            lock_guard<mutex> lock(clientMutex);
            for (SOCKET sock : clientSockets) {
                send(sock, serverMsg.c_str(), serverMsg.length(), 0);
            }
        }
    }
}

int main() {
    cout << "==========================================" << endl;
    cout << "  Task 2: Concurrent Server (Multi-Chat) " << endl;
    cout << "==========================================" << endl;
    
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
    
    // Step 2: Set socket options
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    // Step 3: Configure server address
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    // Step 4: Bind socket
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "[Error] Failed to bind socket to port " << PORT << "!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "[Server] Socket bound to port " << PORT << "." << endl;
    
    // Step 5: Listen for connections (backlog = MAX_CLIENTS for concurrent server)
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        cerr << "[Error] Failed to listen on socket!" << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    cout << "[Server] Listening for up to " << MAX_CLIENTS << " concurrent connections..." << endl;
    cout << "[Server] Server is ready! Waiting for clients..." << endl;
    cout << "------------------------------------------" << endl;
    
    // Start server console thread
    thread consoleThread(serverConsole);
    consoleThread.detach();  // Let console run independently
    
    int nextClientId = 1;  // Client ID counter
    
    // Main loop: Accept new client connections
    while (serverRunning) {
        sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        
        // Accept new connection (blocking call)
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            if (serverRunning) {
                cerr << "[Error] Failed to accept connection!" << endl;
            }
            continue;
        }
        
        // Check if we've reached max clients
        if (clientCount >= MAX_CLIENTS) {
            const char* fullMsg = "[Server] Sorry, server is full. Try again later.";
            send(clientSocket, fullMsg, strlen(fullMsg), 0);
            closesocket(clientSocket);
            continue;
        }
        
        // Get client IP address
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
        
        // Add client to list
        {
            lock_guard<mutex> lock(clientMutex);
            clientSockets.push_back(clientSocket);
            clientCount++;
        }
        
        // Create a new thread to handle this client
        // Thread is detached so it runs independently
        thread clientThread(handleClient, clientSocket, nextClientId, string(clientIP));
        clientThread.detach();
        
        nextClientId++;
    }
    
    // Clean up
    cout << "[Server] Closing server socket..." << endl;
    closesocket(serverSocket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    cout << "[Server] Server shutdown complete. Goodbye!" << endl;
    return 0;
}
