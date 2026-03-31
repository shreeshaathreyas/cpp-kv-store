# cpp-kv-store
cpp-kv-store is a thread-safe key-value store implemented in C++. It features a TCP server that can handle multiple client connections concurrently using a thread pool. The key-value store supports basic operations such as setting, getting, and deleting key-value pairs, along with TTL (time-to-live) expiration for stored keys.

## Project Structure

```
cpp-kv-store
├── src
│   ├── main.cpp        # Entry point of the application
│   ├── server.cpp      # TCP server implementation
│   ├── store.cpp       # Thread-safe key-value store implementation
│   └── parser.cpp      # Command parsing logic
├── include
│   ├── server.h        # Header for the Server class
│   ├── store.h         # Header for the Store class
│   └── parser.h        # Header for the Parser class
├── tests
│   └── kv_store_tests.cpp # Unit tests for the key-value store
├── docs
│   └── design.md       # Design and architecture documentation
├── CMakeLists.txt      # CMake configuration file
└── README.md           # Project documentation
```

## Features

- **Thread-Safe Storage**: Utilizes `unordered_map` with mutexes to ensure safe concurrent access.
- **TTL Expiration**: Supports time-to-live for stored keys, allowing automatic expiration.
- **TCP Server**: Implements a TCP server using POSIX sockets to handle multiple clients.
- **Command Parsing**: Parses commands like `SET`, `GET`, and `DEL` for interacting with the key-value store.

## Setup Instructions

1. Clone the repository:
   ```
   git clone https://github.com/microsoft/vscode-remote-try-cpp.git
   cd cpp-kv-store
   ```

2. Build the project using CMake:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Run the server:
   ```
   ./cpp-kv-store
   ```

## Usage Examples

- To set a key-value pair:
  ```
  SET myKey myValue
  ```

- To get a value by key:
  ```
  GET myKey
  ```

- To delete a key:
  ```
  DEL myKey
  ```

## License

This project is licensed under the MIT License. See the LICENSE file for details.
