/**
 * Task 1: Iterative Client - Chat Application
 *
 * This client connects to the C++ iterative server for one-on-one chat.
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
const char *SERVER_IP = "127.0.0.1"; // localhost for C++ server
const int SERVER_PORT = 8080;        // C++ server port
const char *MY_NAME = "Arjun Rajesh: 23208";
// =======================================================

atomic<bool> running(true);

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
    cout << "\n[Server]: " << buffer << endl;
    cout << "[You]: " << flush;
  }
}

void sendMessages(SOCKET sock) {
  string input;

  while (running) {
    cout << "[You]: " << flush;
    getline(cin, input);

    if (!running)
      break;

    if (input == "exit") {
      cout << "[Client] Disconnecting..." << endl;
      running = false;
      break;
    }

    if (!input.empty()) {
      send(sock, input.c_str(), input.length(), 0);
    }
  }
}

int main() {
  cout << "========================================" << endl;
  cout << "  Task 1: Iterative Client (TCP Chat)  " << endl;
  cout << "========================================" << endl;

#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    cerr << "[Error] WSAStartup failed!" << endl;
    return 1;
  }
#endif

  SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == INVALID_SOCKET) {
    cerr << "[Error] Failed to create socket!" << endl;
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT);

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

  if (connect(clientSocket, (sockaddr *)&serverAddress,
              sizeof(serverAddress)) == SOCKET_ERROR) {
    cerr << "[Error] Connection failed! Is the server running?" << endl;
    closesocket(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  cout << "[Client] Connected to server!" << endl;

  // Send greeting message
  string greeting = string(MY_NAME) + " here!";
  send(clientSocket, greeting.c_str(), greeting.length(), 0);
  cout << "[Client] Sent: " << greeting << endl;

  cout << "[Client] Type 'exit' to disconnect." << endl;
  cout << "----------------------------------------" << endl;

  thread recvThread(receiveMessages, clientSocket);
  thread sendThread(sendMessages, clientSocket);

  recvThread.join();
  sendThread.join();

  closesocket(clientSocket);

#ifdef _WIN32
  WSACleanup();
#endif

  cout << "[Client] Goodbye!" << endl;
  return 0;
}
