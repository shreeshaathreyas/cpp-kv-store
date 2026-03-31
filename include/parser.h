#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <optional>
#include <cstdint>

class Parser {
public:
    enum CommandType { SET, GET, DEL, INVALID };

    struct Command {
        CommandType type = INVALID;
        std::string key;
        std::string value;
        std::optional<uint64_t> ttl_seconds;
    };

    Command parse(const std::string& input);
};

#endif // PARSER_H