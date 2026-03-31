cmake_minimum_required(VERSION 3.10)
project(cpp_kv_store)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(cpp-kv-store
    src/main.cpp
    src/server.cpp
    src/store.cpp
    src/parser.cpp
)

# ✅ Platform-specific linking
if(WIN32)
    target_link_libraries(cpp-kv-store ws2_32)
else()
    target_link_libraries(cpp-kv-store pthread)
endif()