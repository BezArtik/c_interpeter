// token.cpp

// This file implements the token-related functions declared in token.hpp.


#include "core/token.hpp"
#include <algorithm>

namespace core {

std::optional<token_type> lookup_keyword(std::string_view lexeme) {
	auto it = std::find_if(keyword_table.begin(), keyword_table.end(),
		[lexeme](const auto& kw) { return kw.lexeme_ == lexeme; });
	return it != keyword_table.end() ? std::make_optional(it->token_) : std::nullopt;
}

std::optional<value_type> token_to_value_type(token_type t) {
	auto it = std::find_if(keyword_table.begin(), keyword_table.end(),
		[t](const auto& kw) { return kw.token_ == t; });
	return it != keyword_table.end() ? it->type_ : std::nullopt;
}

bool can_start_statement(token_type t) {
	auto it = std::find_if(keyword_table.begin(), keyword_table.end(),
		[t](const auto& kw) { return kw.token_ == t; });
	return it != keyword_table.end() && it->can_start_statement_;
}

bool is_double_literal(std::string_view lexeme) {
    return lexeme.find('.') != std::string_view::npos;
}

} // namespace core