#include <gtest/gtest.h>
#include "../include/store.h"

class KVStoreTests : public ::testing::Test {
protected:
    Store store;

    void SetUp() override {
        // Optional: Code to set up before each test
    }

    void TearDown() override {
        // Optional: Code to clean up after each test
    }
};

TEST_F(KVStoreTests, SetAndGet) {
    store.set("key1", "value1");
    EXPECT_EQ(store.get("key1"), "value1");
}

TEST_F(KVStoreTests, DeleteKey) {
    store.set("key2", "value2");
    store.del("key2");
    EXPECT_EQ(store.get("key2"), "");
}

TEST_F(KVStoreTests, TTLExpiration) {
    store.set("key3", "value3", 1); // Set with TTL of 1 second
    EXPECT_EQ(store.get("key3"), "value3");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(store.get("key3"), "");
}

TEST_F(KVStoreTests, OverwriteValue) {
    store.set("key4", "value4");
    store.set("key4", "new_value4");
    EXPECT_EQ(store.get("key4"), "new_value4");
}

TEST_F(KVStoreTests, NonExistentKey) {
    EXPECT_EQ(store.get("non_existent_key"), "");
}