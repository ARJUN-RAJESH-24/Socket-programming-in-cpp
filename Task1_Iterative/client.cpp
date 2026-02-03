/**
 * Task 1: C++ Client for Python Chat Server (Iterative)
 *
 * This client connects to a Python chat server for one-on-one chat.
 * Compatible with Python socket server.
 *
 * Compile (Windows): g++ -o client.exe client.cpp -lws2_32
 *
 * Course: 23CSE312 - Distributed Systems
 * Lab: Socket Programming (Chat Application)
 */

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
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
#include <string>
#include <thread>


using namespace std;

// ============ CONFIGURATION - CHANGE THESE ============
const char *SERVER_IP = "10.105.10.247";     // Your friend's server IP
const int SERVER_PORT = 12345;               // Python server port
const char *MY_NAME = "Arjun Rajesh: 23208"; // Your name
// =======================================================

atomic<bool> running(true);

/**
 * Function: receiveMessages
 * Continuously receives messages from the Python server
 */
void receiveMessages(SOCKET sock) {
  char buffer[1024];

  while (running) {
    memset(buffer, 0, sizeof(buffer));

    int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
      if (running) {
        cout << "\n[Disconnected from server]" << endl;
      }
      running = false;
      break;
    }

    buffer[bytesRead] = '\0';
    cout << "\r" << buffer << endl;
    cout << "-> " << flush;
  }
}

/**
 * Function: sendMessages
 * Sends messages to the Python server with name prefix
 */
void sendMessages(SOCKET sock) {
  string input;

  while (running) {
    cout << "-> " << flush;
    getline(cin, input);

    if (!running)
      break;

    if (input == "exit" || input == "EXIT") {
      string exitMsg = string(MY_NAME) + " has left the chat.";
      send(sock, exitMsg.c_str(), exitMsg.length(), 0);
      cout << "[Disconnecting...]" << endl;
      running = false;
      break;
    }

    if (!input.empty()) {
      // Format: "Arjun Rajesh: 23208: message"
      string fullMessage = string(MY_NAME) + ": " + input;
      send(sock, fullMessage.c_str(), fullMessage.length(), 0);
    }
  }
}

int main() {
  cout << "==========================================" << endl;
  cout << "  Task 1: C++ Client for Python Server   " << endl;
  cout << "==========================================" << endl;
  cout << "Name: " << MY_NAME << endl;
  cout << "Server: " << SERVER_IP << ":" << SERVER_PORT << endl;
  cout << "------------------------------------------" << endl;

#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    cerr << "[Error] WSAStartup failed!" << endl;
    return 1;
  }
#endif

  // Create TCP socket
  SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == INVALID_SOCKET) {
    cerr << "[Error] Failed to create socket!" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  // Configure server address
  sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT);

  if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
    cerr << "[Error] Invalid server address: " << SERVER_IP << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  cout << "[Connecting to " << SERVER_IP << ":" << SERVER_PORT << "...]"
       << endl;

  // Connect to Python server
  if (connect(clientSocket, (sockaddr *)&serverAddress,
              sizeof(serverAddress)) == SOCKET_ERROR) {
    cerr << "[Error] Connection failed! Is the server running?" << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  cout << "--- Connected as " << MY_NAME << " ---" << endl;

  // Send name to server first (required by Python server)
  send(clientSocket, MY_NAME, strlen(MY_NAME), 0);

  cout << "[Type messages and press Enter. Type 'exit' to leave]" << endl;
  cout << "------------------------------------------" << endl;

  // Start receive and send threads
  thread recvThread(receiveMessages, clientSocket);
  thread sendThread(sendMessages, clientSocket);

  recvThread.join();
  sendThread.join();

  // Cleanup
  closesocket(clientSocket);

#ifdef _WIN32
  WSACleanup();
#endif

  cout << "[Client closed]" << endl;
  return 0;
}
