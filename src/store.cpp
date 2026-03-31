#include "store.h"
#include <chrono>

KVStore::KVStore() : running_(true), cleaner_(&KVStore::cleanupLoop, this) {}

KVStore::~KVStore() {
    running_ = false;
    cv_.notify_all();
    if (cleaner_.joinable()) {
        cleaner_.join();
    }
}

void KVStore::set(const std::string& key, const std::string& value, uint64_t ttl_seconds) {
    std::lock_guard<std::mutex> g(lock_);
    Value v;
    v.data = value;
    v.has_ttl = (ttl_seconds > 0);
    v.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    store_[key] = std::move(v);
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> g(lock_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return std::nullopt;
    }
    if (isExpired(it->second)) {
        store_.erase(it);
        return std::nullopt;
    }
    return it->second.data;
}

bool KVStore::del(const std::string& key) {
    std::lock_guard<std::mutex> g(lock_);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    store_.erase(it);
    return true;
}

bool KVStore::isExpired(const Value& v) const {
    return v.has_ttl && std::chrono::steady_clock::now() >= v.expires_at;
}

void KVStore::removeExpiredLocked() {
    for (auto it = store_.begin(); it != store_.end();) {
        if (isExpired(it->second)) {
            it = store_.erase(it);
        } else {
            ++it;
        }
    }
}

void KVStore::cleanupLoop() {
    std::unique_lock<std::mutex> g(lock_, std::defer_lock);
    while (running_) {
        {
            std::lock_guard<std::mutex> lg(lock_);
            removeExpiredLocked();
        }
        std::unique_lock<std::mutex> ul(lock_);
        cv_.wait_for(ul, std::chrono::seconds(1), [this] { return !running_; });
    }
}