# Custom HTTP/1.1 & TCP Server in C

This project contains two simple implementations in pure C:
1. **Basic HTTP/1.1 Server** — Handles simple HTTP GET requests.
2. **TCP Server** — A minimal TCP Echo server demonstrating basic socket communication.

---

## 📂 Project Structure

.
├── Basic-HTTP-Server/
│ ├── http_server.c
│ └── (other HTTP server files)
├── TCP-Server/
│ ├── tcp_server.c
│ └── (other TCP server files)
└── README.md

yaml
Copy
Edit

---

## 🚀 How to Run

### Prerequisites:
- GCC Compiler
- Linux (Tested on Kali Linux & WSL)
- Basic knowledge of networking sockets

---

### 1. Compile and Run HTTP Server
```bash
cd Basic-HTTP-Server
gcc http_server.c -o http_server
./http_server
2. Compile and Run TCP Server
bash
Copy
Edit
cd TCP-Server
gcc tcp_server.c -o tcp_server
./tcp_server
📡 Features
Basic HTTP/1.1 Server
Accepts HTTP GET requests.

Sends back hardcoded HTTP/1.1 response.

Demonstrates raw socket programming for HTTP.

TCP Server
Basic Echo Server.

Handles simple TCP client connections.

Useful for learning raw socket data transmission.

🛠️ To-Do (Optional Enhancements)
Add multi-client support (fork/thread/poll/epoll).

Implement proper HTTP request parsing.

Add HTTP Response Headers dynamically.

Handle errors & edge cases more gracefully.
