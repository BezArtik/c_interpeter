#pragma once

#include <cstdint>
#include <string_view>

namespace core {

enum class token_type : uint8_t {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    IF, ELSE, WHILE, RETURN,
    INT_KEYWORD, DOUBLE_KEYWORD, BOOL_KEYWORD,
    TRUE, FALSE,

    END_OF_FILE
};


struct token {
    token_type type_;
    std::string_view lexeme_;
    size_t line_;
    size_t column_;
};

}
