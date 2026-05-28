// core/token_types.hpp

// This file defines the token types and 
// value types used in the core namespace.


#pragma once
#include "core/token/token.hpp"

namespace core {

enum class token_type : uint8_t {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    PERCENT, INCREMENT, DECREMENT,
    PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    IF, ELSE, WHILE, FOR, RETURN,
    INT_KEYWORD, DOUBLE_KEYWORD, BOOL_KEYWORD, STRING_KEYWORD, VOID_KEYWORD,
    TRUE, FALSE,
    AND, OR,

    END_OF_FILE,
    UNKNOWN
};

enum class value_type : uint8_t {
    INT, DOUBLE, BOOL, STRING, VOID, UNKNOWN
};

} // namespace core