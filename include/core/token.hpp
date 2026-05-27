// core/token.hpp

// This file contains forward declarations 
// for token-related types in the core namespace.


#pragma once
#include <cstdint>
#include <string_view>

namespace core {

enum class token_type : uint8_t;
enum class value_type : uint8_t;

struct token {
    token_type type_;
    std::string_view lexeme_;
    size_t line_;
    size_t column_;
};

} // namespace core