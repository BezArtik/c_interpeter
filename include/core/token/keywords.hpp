// core/keywords.hpp

// This file defines the keyword structure 
// and related functions for the core namespace.


#pragma once
#include "core/token/token_types.hpp"
#include <cstdint>
#include <string_view>
#include <array>
#include <optional>

namespace core {

struct keyword_info {
    std::string_view lexeme_;
    token_type token_;
    std::optional<value_type> type_;
    bool can_start_statement_;
};

inline constexpr std::array keyword_table{
    keyword_info{"if",      token_type::IF,             std::nullopt,       true},
    keyword_info{"else",    token_type::ELSE,           std::nullopt,       false},
    keyword_info{"while",   token_type::WHILE,          std::nullopt,       true},
    keyword_info{"for",     token_type::FOR,            std::nullopt,       true},
    keyword_info{"return",  token_type::RETURN,         std::nullopt,       true},
    keyword_info{"int",     token_type::INT_KEYWORD,    value_type::INT,    true},
    keyword_info{"double",  token_type::DOUBLE_KEYWORD, value_type::DOUBLE, true},
    keyword_info{"bool",    token_type::BOOL_KEYWORD,   value_type::BOOL,   true},
    keyword_info{"string",  token_type::STRING_KEYWORD, value_type::STRING, true},
    keyword_info{"void",    token_type::VOID_KEYWORD,   value_type::VOID,   true},
    keyword_info{"true",    token_type::TRUE,           value_type::BOOL,   false},
    keyword_info{"false",   token_type::FALSE,          value_type::BOOL,   false},
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
	token_type::FOR,
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

} // namespace core
