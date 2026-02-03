# Socket Programming Lab - Distributed Systems (23CSE312)

## 📌 Overview
This repository contains the implementation of **TCP Socket Programming** for a chat application, completed as part of Lab 1 for the Distributed Systems course.

## 🎯 Tasks Completed

### Task 1: Iterative Server and Client
Located in `Task1_Iterative/`
- **server.cpp** - Handles single client connection at a time (iterative/sequential)
- **client.cpp** - Connects to the server for one-on-one chat

### Task 2: Concurrent Server and Client  
Located in `Task2_Concurrent/`
- **server.cpp** - Handles multiple clients simultaneously using multi-threading
- **client.cpp** - Participates in group chat with other connected clients

---

## 🔧 Compilation Instructions

### Windows (using MinGW g++)
```powershell
# Task 1 - Iterative
cd Task1_Iterative
g++ -o server.exe server.cpp -lws2_32
g++ -o client.exe client.cpp -lws2_32

# Task 2 - Concurrent
cd Task2_Concurrent
g++ -o server.exe server.cpp -lws2_32
g++ -o client.exe client.cpp -lws2_32
```

### Linux
```bash
# Task 1 - Iterative
cd Task1_Iterative
g++ -o server server.cpp -pthread
g++ -o client client.cpp -pthread

# Task 2 - Concurrent
cd Task2_Concurrent
g++ -o server server.cpp -pthread
g++ -o client client.cpp -pthread
```

---

## 🚀 Running the Applications

### Task 1: Iterative Chat (One-on-One)

1. **Start the server** (Terminal 1):
   ```
   ./server.exe    # Windows
   ./server        # Linux
   ```

2. **Start the client** (Terminal 2):
   ```
   ./client.exe    # Windows
   ./client        # Linux
   ```

3. **Chat!** Type messages and press Enter. Type `exit` to disconnect.

### Task 2: Concurrent Chat (Group Chat)

1. **Start the server** (Terminal 1):
   ```
   ./server.exe    # Windows
   ./server        # Linux
   ```

2. **Start multiple clients** (Terminal 2, 3, 4, ...):
   ```
   ./client.exe    # Windows
   ./client        # Linux
   ```

3. **Group Chat!** Messages from any client are broadcast to all other clients.
4. Server can type `quit` to shutdown and disconnect all clients.

---

## 📡 Cross-System Testing (Same Network)

To test between two machines on the same LAN/Wi-Fi:

1. Find the server's IP address:
   - Windows: `ipconfig`
   - Linux: `ip addr` or `hostname -I`

2. Modify `SERVER_IP` in the client code:
   ```cpp
   const char* SERVER_IP = "192.168.x.x";  // Server's IP
   ```

3. Ensure firewall allows port 8080.

---

## 📋 Features

| Feature | Task 1 (Iterative) | Task 2 (Concurrent) |
|---------|-------------------|---------------------|
| Connection Type | Single client | Multiple clients |
| Threading | Per-client I/O | Per-client handler |
| Client Limit | 1 at a time | Up to 10 simultaneous |
| Message Type | Direct chat | Broadcast to all |
| Server Console | No | Yes (broadcast messages) |

---

## 🏗️ Architecture

### Client-Server Model

```
┌─────────────────────────────────────────────────────────────┐
│                        SERVER                                │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  1. Create Socket (socket)                          │    │
│  │  2. Bind to Port (bind)                             │    │
│  │  3. Listen for Connections (listen)                 │    │
│  │  4. Accept Clients (accept)                         │    │
│  │  5. Send/Receive Data (send/recv)                   │    │
│  │  6. Close Connection (close)                        │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                        CLIENT                                │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  1. Create Socket (socket)                          │    │
│  │  2. Connect to Server (connect)                     │    │
│  │  3. Send/Receive Data (send/recv)                   │    │
│  │  4. Close Connection (close)                        │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## 📚 Key Concepts

- **TCP Socket**: Reliable, connection-oriented communication
- **Socket API**: `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`
- **Iterative Server**: Serves one client at a time (sequential)
- **Concurrent Server**: Serves multiple clients simultaneously (multi-threaded)
- **Threading**: Separate threads for sending and receiving messages

---

## 📝 Submission Checklist

- [x] Task 1: server.cpp (Iterative Server)
- [x] Task 1: client.cpp (Iterative Client)
- [x] Task 2: server.cpp (Concurrent Server)
- [x] Task 2: client.cpp (Concurrent Client)
- [ ] Screenshots of output from both server and client
- [x] Well-documented code with comments

---

## 👨‍💻 Course Information

- **Course Code**: 23CSE312
- **Course Name**: Distributed Systems
- **Lab**: Lab 1 - Socket Programming
- **Topic**: Distributed Applications using Sockets (Chat Program)