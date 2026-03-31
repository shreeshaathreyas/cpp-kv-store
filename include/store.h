#ifndef STORE_H
#define STORE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <thread>
#include <condition_variable>
#include <atomic>

class KVStore {
public:
    KVStore();
    ~KVStore();

    void set(const std::string& key, const std::string& value, uint64_t ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);

private:
    struct Value {
        std::string data;
        std::chrono::steady_clock::time_point expires_at;
        bool has_ttl;
    };

    bool isExpired(const Value& v) const;
    void cleanupLoop();
    void removeExpiredLocked();

    std::unordered_map<std::string, Value> store_;
    mutable std::mutex lock_;
    std::thread cleaner_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
};

#endif // STORE_H