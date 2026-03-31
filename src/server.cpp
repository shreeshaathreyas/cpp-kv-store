#include "server.h"
#include <iostream>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

KVServer::KVServer(KVStore& store, Parser& parser)
    : store_(store), parser_(parser), running_(false) {}

KVServer::~KVServer() {
    stop();
}

void KVServer::start(uint16_t port, size_t worker_count) 
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif

    if (running_) return;
    running_ = true;

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("socket failed");
    }

    int opt = 1;

#ifdef _WIN32
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
    closesocket(server_fd_);
    WSACleanup();
#else
    close(sock);
#endif
    throw std::runtime_error("bind failed");
    }

    if (listen(server_fd_, 16) < 0) {
#ifdef _WIN32
    closesocket(server_fd_);
    WSACleanup();
#else
    close(sock);
#endif
        throw std::runtime_error("listen failed");
    }

    accept_thread_ = std::thread([this, port]() { acceptLoop(port); });

    for (size_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back([this]() { workerLoop(); });
    }
}

void KVServer::stop() {
    if (!running_) return;
    running_ = false;
    if (server_fd_ >= 0) {
#ifdef _WIN32
    closesocket(server_fd_);
    WSACleanup();
#else
    close(sock);
#endif
        server_fd_ = -1;
    }
    queue_cv_.notify_all();
    if (accept_thread_.joinable()) accept_thread_.join();
    {
        std::lock_guard<std::mutex> lg(queue_mtx_);
        while (!tasks_.empty()) {
#ifdef _WIN32
            closesocket(tasks_.front());
            WSACleanup();
#else
            close(sock);
#endif
            tasks_.pop();
        }
    }
    for (auto& t : workers_) {
        if (t.joinable()) t.join();
    }
    workers_.clear();
}

void KVServer::acceptLoop(uint16_t /*port*/) {
    while (running_) {
        int clientfd = accept(server_fd_, nullptr, nullptr);
        if (clientfd < 0) {
            if (running_) continue;
            break;
        }
        {
            std::lock_guard<std::mutex> lg(queue_mtx_);
            tasks_.push(clientfd);
        }
        queue_cv_.notify_one();
    }
}

void KVServer::workerLoop() {
    while (running_) {
        int clientfd = -1;
        {
            std::unique_lock<std::mutex> ul(queue_mtx_);
            queue_cv_.wait(ul, [this] { return !running_ || !tasks_.empty(); });
            if (!running_ && tasks_.empty()) return;
            if (!tasks_.empty()) {
                clientfd = tasks_.front();
                tasks_.pop();
            }
        }
        if (clientfd >= 0) {
            handleClient(clientfd);
#ifdef _WIN32
            closesocket(clientfd);
            WSACleanup();
#else
            close(clientfd);
#endif
        }
    }
}

static bool readLine(int fd, std::string& out) {
    out.clear();
    while (true) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) return false;
        if (c == '\n') break;
        if (c == '\r') continue;
        out.push_back(c);
    }
    return true;
}

static void sendReply(int fd, const std::string& msg) {
    std::string out = msg;
    if (!out.empty() && out.back() != '\n') out.push_back('\n');
    send(fd, out.data(), out.size(), 0);
}

void KVServer::handleClient(int clientfd) {
    std::string line;
    while (readLine(clientfd, line)) {
        if (line.empty()) continue;
        Parser::Command cmd;
        try {
            cmd = parser_.parse(line);
        } catch (const std::exception& e) {
            sendReply(clientfd, std::string("ERROR ") + e.what());
            continue;
        }

        switch (cmd.type) {
        case Parser::SET:
            if (cmd.ttl_seconds.has_value()) {
                store_.set(cmd.key, cmd.value, *cmd.ttl_seconds);
            } else {
                store_.set(cmd.key, cmd.value);
            }
            sendReply(clientfd, "OK");
            break;
        case Parser::GET: {
            auto val = store_.get(cmd.key);
            if (val.has_value()) {
                sendReply(clientfd, *val);
            } else {
                sendReply(clientfd, "(nil)");
            }
            break;
        }
        case Parser::DEL: {
            bool removed = store_.del(cmd.key);
            sendReply(clientfd, removed ? "1" : "0");
            break;
        }
        default:
            sendReply(clientfd, "ERROR invalid command");
            break;
        }
    }
}