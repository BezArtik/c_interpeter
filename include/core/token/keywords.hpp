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
    bool can_start_statement_;
};

inline constexpr std::array keyword_table{
    keyword_info{"if",      token_type::IF,             true},
    keyword_info{"else",    token_type::ELSE,           false},
    keyword_info{"while",   token_type::WHILE,          true},
    keyword_info{"for",     token_type::FOR,            true},
    keyword_info{"return",  token_type::RETURN,         true},
    keyword_info{"int",     token_type::INT_KEYWORD,    true},
    keyword_info{"double",  token_type::DOUBLE_KEYWORD, true},
    keyword_info{"bool",    token_type::BOOL_KEYWORD,   true},
    keyword_info{"string",  token_type::STRING_KEYWORD, true},
    keyword_info{"void",    token_type::VOID_KEYWORD,   true},
    keyword_info{"true",    token_type::TRUE,           false},
    keyword_info{"false",   token_type::FALSE,          false},
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
std::optional<type> token_to_type(token_type t);
bool can_start_statement(token_type t);
bool is_double_literal(std::string_view lexeme);

} // namespace core
