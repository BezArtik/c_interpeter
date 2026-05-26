// This file defines the abstract syntax tree (AST) 
// nodes for expressions in the language.


#pragma once

#include "core/token.hpp"
#include <variant>
#include <memory>
#include <vector>

namespace ast {

struct literal_expr {
    core::token value_;
};

struct variable_expr {
    core::token name_;
};

struct binary_expr {
    std::unique_ptr<struct expression> left_;
    core::token op_;
    std::unique_ptr<struct expression> right_;
};

struct unary_expr {
    core::token op_;
    std::unique_ptr<struct expression> operand_;
};

struct postfix_expr {
	std::unique_ptr<struct expression> operand_;
	core::token op_;
};

struct call_expr {
    core::token callee_;
    std::vector<std::unique_ptr<struct expression>> args_;
};

struct expression {
    std::variant<
        literal_expr,
        variable_expr,
        binary_expr,
        unary_expr,
        postfix_expr,
        call_expr
    > data_;

    expression();
    expression(literal_expr e);
    expression(variable_expr e);
    expression(binary_expr e);
    expression(unary_expr e);
    expression(postfix_expr e);
    expression(call_expr e);
};

} // namespace ast