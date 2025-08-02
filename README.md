# Custom HTTP/1.1 & TCP Server in C

This project contains two simple implementations in pure C:
1. **Basic HTTP/1.1 Server** — Handles simple HTTP GET requests.
2. **TCP Server** — A minimal TCP Echo server demonstrating basic socket communication.

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
