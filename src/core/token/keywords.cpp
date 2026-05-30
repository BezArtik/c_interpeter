// core/keywords.cpp

// This file implements the keyword-related 
// functions declared in keywords.hpp.


#include "core/token/token_types.hpp"
#include "core/token/keywords.hpp"
#include <algorithm>
#include <optional>
#include <string_view>

namespace core {

std::optional<token_type> lookup_keyword(std::string_view lexeme) {
    auto it = std::ranges::find(keyword_table, lexeme, &keyword_info::lexeme_);
    if (it != keyword_table.end()) return it->token_;
    return std::nullopt;
}

std::optional<type> token_to_type(token_type t) {
    switch (t) {
    case token_type::INT_KEYWORD:    return type::int_type();
    case token_type::DOUBLE_KEYWORD: return type::double_type();
    case token_type::BOOL_KEYWORD:   return type::bool_type();
    case token_type::STRING_KEYWORD: return type::string_type();
    case token_type::VOID_KEYWORD:   return type::void_type();
    default: return std::nullopt;
    }
}

bool can_start_statement(token_type t) {
    auto it = std::ranges::find(keyword_table, t, &keyword_info::token_);
    return it != keyword_table.end() && it->can_start_statement_;
}

bool is_double_literal(std::string_view lexeme) {
    return lexeme.find('.') != std::string_view::npos;
}

} // namespace core