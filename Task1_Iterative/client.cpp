/**
 * Task 1: Iterative Client - Chat Application
 *
 * This client connects to the iterative server and enables
 * bidirectional chat communication over TCP sockets.
 *
 * Compile (Windows): g++ -o client.exe client.cpp -lws2_32
 * Compile (Linux):   g++ -o client client.cpp -pthread
 *
 * Course: 23CSE312 - Distributed Systems
 * Lab: Socket Programming (Chat Application)
 */

#ifdef _WIN32
// Windows-specific headers for socket programming
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
// Linux/Unix-specific headers for socket programming
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>


using namespace std;

// Global flag to signal threads to stop
atomic<bool> running(true);

// Server configuration - modify these for cross-system testing
const char *SERVER_IP =
    "127.0.0.1"; // Change to server's IP for network testing
const int SERVER_PORT = 8080;

/**
 * Function: receiveMessages
 * Purpose: Continuously receives messages from the server
 * Parameters: sock - The socket descriptor for communication
 *
 * This function runs in a separate thread and displays incoming
 * messages from the server. It terminates when the server disconnects.
 */
void receiveMessages(SOCKET sock) {
  char buffer[1024]; // Buffer to store incoming messages

  while (running) {
    memset(buffer, 0, sizeof(buffer)); // Clear buffer before receiving

    // Receive data from server (blocking call)
    int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
      // Server disconnected or error occurred
      cout << "\n[Client] Server disconnected." << endl;
      running = false;
      break;
    }

    buffer[bytesRead] = '\0'; // Null-terminate the received string
    cout << "\n[Server]: " << buffer << endl;
    cout << "[You]: " << flush; // Prompt for next message
  }
}

/**
 * Function: sendMessages
 * Purpose: Continuously sends messages to the server
 * Parameters: sock - The socket descriptor for communication
 *
 * This function runs in a separate thread and reads user input
 * to send messages to the server. Type "exit" to disconnect.
 */
void sendMessages(SOCKET sock) {
  char buffer[1024]; // Buffer to store outgoing messages

  while (running) {
    cout << "[You]: " << flush;
    cin.getline(buffer, sizeof(buffer)); // Read user input

    if (!running)
      break; // Check if other thread signaled to stop

    // Check for exit command
    if (strcmp(buffer, "exit") == 0) {
      cout << "[Client] Disconnecting..." << endl;
      running = false;
      break;
    }

    // Send message to server
    send(sock, buffer, strlen(buffer), 0);
  }
}

int main() {
  cout << "========================================" << endl;
  cout << "   Task 1: Iterative Client (TCP Chat) " << endl;
  cout << "========================================" << endl;

#ifdef _WIN32
  // Initialize Winsock (Windows only)
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    cerr << "[Error] WSAStartup failed!" << endl;
    return 1;
  }
  cout << "[Client] Winsock initialized successfully." << endl;
#endif

  // Step 1: Create a TCP socket
  SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == INVALID_SOCKET) {
    cerr << "[Error] Failed to create socket!" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }
  cout << "[Client] Socket created successfully." << endl;

  // Step 2: Configure server address structure
  sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;          // IPv4
  serverAddress.sin_port = htons(SERVER_PORT); // Server port

  // Convert IP address from text to binary form
  if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
    cerr << "[Error] Invalid server address!" << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  cout << "[Client] Connecting to " << SERVER_IP << ":" << SERVER_PORT << "..."
       << endl;

  // Step 3: Connect to the server
  if (connect(clientSocket, (sockaddr *)&serverAddress,
              sizeof(serverAddress)) == SOCKET_ERROR) {
    cerr << "[Error] Connection failed! Make sure the server is running."
         << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  cout << "[Client] Connected to server successfully!" << endl;

  // Send greeting message automatically upon connection
  const char *greeting = "Arjun Rajesh -23208 here!";
  send(clientSocket, greeting, strlen(greeting), 0);
  cout << "[Client] Sent greeting: " << greeting << endl;

  cout << "[Client] Chat session started. Type 'exit' to disconnect." << endl;
  cout << "----------------------------------------" << endl;

  // Step 4: Create threads for sending and receiving messages
  thread recvThread(receiveMessages, clientSocket);
  thread sendThread(sendMessages, clientSocket);

  // Wait for both threads to complete
  recvThread.join();
  sendThread.join();

  // Step 5: Clean up - close socket
  cout << "[Client] Shutting down..." << endl;
  closesocket(clientSocket);

#ifdef _WIN32
  WSACleanup(); // Clean up Winsock
#endif

  cout << "[Client] Goodbye!" << endl;
  return 0;
}
