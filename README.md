# Custom HTTP/1.1 & TCP Server in C

This project contains two simple implementations in pure C:
1. **Basic HTTP/1.1 Server** — Handles simple HTTP GET requests. You can request the filename with extension that is staying on the server machine. Also, you can whitelist your ip in txt file and specify the filename when starting the application. Don't worry, the HTTP server will hide this file by returning 404 if the hacker is trying to get it.
2. **TCP Server** — A minimal TCP Echo server demonstrating basic socket communication.

## 🚀 How to Run

### Prerequisites:
- GCC Compiler
- Linux (Tested on Kali Linux & WSL)
- Basic knowledge of networking sockets

---

### 1. Compile and Run HTTP Server
```bash
cd HTTPclient
gcc http_server.c -o http_server
./http_server <path-to-ip-whitelist-txt-file>
