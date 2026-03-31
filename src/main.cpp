#include "server.h"
#include "store.h"
#include "parser.h"
#include <iostream>

int main() {
    KVStore store;
    Parser parser;
    KVServer server(store, parser);

    try {
        server.start(4000, 4);
        std::cout << "Server running on port 4000. Ctrl+C to stop.\n";
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Failed to start server: " << ex.what() << "\n";
        return 1;
    }

    server.stop();
    return 0;
}