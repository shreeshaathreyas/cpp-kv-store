#ifndef SERVER_H
#define SERVER_H

#include "store.h"
#include "parser.h"

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class KVServer {
public:
    KVServer(KVStore& store, Parser& parser);
    ~KVServer();

    void start(uint16_t port = 4000, size_t worker_count = 4);
    void stop();

private:
    void acceptLoop(uint16_t port);
    void workerLoop();
    void handleClient(int clientfd);

    KVStore& store_;
    Parser& parser_;

    std::thread accept_thread_;
    std::vector<std::thread> workers_;
    std::queue<int> tasks_;
    std::mutex queue_mtx_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_;
    int server_fd_ = -1;
};

#endif // SERVER_H