#pragma once

#include "core/type.hpp"
#include <cstdint>
#include <string_view>
#include <array>
#include <optional>

namespace core {

enum class token_type : uint8_t {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
    PERCENT,

    IDENTIFIER, STRING, NUMBER,

    IF, ELSE, WHILE, RETURN,
    INT_KEYWORD, DOUBLE_KEYWORD, BOOL_KEYWORD, STRING_KEYWORD, VOID_KEYWORD,
    TRUE, FALSE,
    AND, OR,

    END_OF_FILE
};


struct token {
    token_type type_;
    std::string_view lexeme_;
    size_t line_;
    size_t column_;
};

struct keyword_info {
    std::string_view lexeme;
    token_type token;
    std::optional<value_type> type;
    bool can_start_statement;
};

inline constexpr std::array keyword_table{
    keyword_info{"if",      token_type::IF,            std::nullopt,             true},
    keyword_info{"else",    token_type::ELSE,          std::nullopt,             false},
    keyword_info{"while",   token_type::WHILE,         std::nullopt,             true},
    keyword_info{"return",  token_type::RETURN,        std::nullopt,             true},
    keyword_info{"int",     token_type::INT_KEYWORD,   core::value_type::INT,    true},
    keyword_info{"double",  token_type::DOUBLE_KEYWORD,core::value_type::DOUBLE, true},
    keyword_info{"bool",    token_type::BOOL_KEYWORD,  core::value_type::BOOL,   true},
    keyword_info{"string",  token_type::STRING_KEYWORD,core::value_type::STRING, true},
    keyword_info{"void",    token_type::VOID_KEYWORD,  core::value_type::VOID,   true},
    keyword_info{"true",    token_type::TRUE,          core::value_type::BOOL,   false},
    keyword_info{"false",   token_type::FALSE,         core::value_type::BOOL,   false},
};

inline constexpr std::array type_tokens{
    token_type::INT_KEYWORD,
    token_type::DOUBLE_KEYWORD,
    token_type::BOOL_KEYWORD,
    token_type::STRING_KEYWORD,
    token_type::VOID_KEYWORD,
};

inline constexpr std::array statement_starters{
    token_type::IF,
    token_type::WHILE,
    token_type::RETURN,
    token_type::INT_KEYWORD,
    token_type::DOUBLE_KEYWORD,
    token_type::BOOL_KEYWORD,
    token_type::STRING_KEYWORD,
    token_type::VOID_KEYWORD,
};

std::optional<token_type> lookup_keyword(std::string_view lexeme);
std::optional<value_type> token_to_value_type(token_type t);
bool can_start_statement(token_type t);
bool is_double_literal(std::string_view lexeme);

}
