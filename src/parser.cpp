#include "parser.h"
#include <sstream>
#include <stdexcept>
#include <cstdint>

Parser::Command Parser::parse(const std::string& input) {
    std::istringstream stream(input);
    std::string cmd;
    stream >> cmd;

    Command result;
    if (cmd == "SET") {
        result.type = SET;
        stream >> result.key >> result.value;
        if (stream.fail() || result.key.empty()) {
            throw std::runtime_error("Invalid SET syntax");
        }
        uint64_t ttl = 0;
        if (stream >> ttl) {
            if (ttl == 0) {
                throw std::runtime_error("TTL must be >0");
            }
            result.ttl_seconds = ttl;
        }
    } else if (cmd == "GET") {
        result.type = GET;
        stream >> result.key;
        if (stream.fail() || result.key.empty()) {
            throw std::runtime_error("Invalid GET syntax");
        }
    } else if (cmd == "DEL") {
        result.type = DEL;
        stream >> result.key;
        if (stream.fail() || result.key.empty()) {
            throw std::runtime_error("Invalid DEL syntax");
        }
    } else {
        result.type = INVALID;
    }

    return result;
}