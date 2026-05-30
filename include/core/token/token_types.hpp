// core/token_types.hpp

// This file defines the token types and 
// value types used in the core namespace.


#pragma once
#include "core/token/token.hpp"
#include <vector>
#include <memory>
#include <variant>

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


class type {
public:

    type() = default;
    type(const type& other);
    type& operator=(const type& other);
    type(type&&) = default;
    type& operator=(type&&) = default;
    
    static type int_type();
    static type double_type();
    static type bool_type();
    static type string_type();
    static type void_type();
    static type unknown_type();

    static type function_type(type return_type, std::vector<type> param_types);

    bool is_primitive() const noexcept;
    bool is_numeric() const noexcept;
    bool is_void() const noexcept;
    bool is_unknown() const noexcept;
    bool is_function() const noexcept;

    const type& return_type() const;
    const std::vector<type>& param_types() const;

    bool is_assignable_from(const type& source) const noexcept;
    type common_arithmetic_type(const type& other) const noexcept;

    bool operator==(const type& other) const noexcept;
    bool operator!=(const type& other) const noexcept;

private:
    
    enum class kind : uint8_t {
        INT, DOUBLE, BOOL, STRING, VOID,
        FUNCTION,
        UNKNOWN
    };

    struct function_info {
        std::unique_ptr<type> return_type;
        std::vector<type> param_types;
    };

    type(kind k);
    type(kind k, function_info info);
    void swap(type& other) noexcept;

    kind kind_ = kind::UNKNOWN;
    std::variant<
        std::monostate,
        function_info
    > info_;
};

} // namespace core