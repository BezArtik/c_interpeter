#include "core/token.hpp"

namespace core {

std::optional<token_type> lookup_keyword(std::string_view lexeme) {
    for (const auto& kw : keyword_table) {
        if (kw.lexeme == lexeme) return kw.token;
    }
    return std::nullopt;
}

std::optional<value_type> token_to_value_type(token_type t) {
    for (const auto& kw : keyword_table) {
        if (kw.token == t) return kw.type;
    }
    return std::nullopt;
}

bool can_start_statement(token_type t) {
    for (const auto& kw : keyword_table) {
        if (kw.token == t) return kw.can_start_statement;
    }
    return false;
}

bool is_double_literal(std::string_view lexeme) {
    return lexeme.find('.') != std::string_view::npos;
}

}