// ast/expression.hpp

// This file defines the abstract syntax tree (AST) 
// nodes for expressions in the language.


#pragma once

#include "core/token/token.hpp"
#include <variant>
#include <memory>
#include <vector>

namespace ast {

struct expression;

using expr_ptr = std::unique_ptr<expression>;

struct literal_expr {
    core::token value_{};
    size_t line_{};
    size_t column_{};

	literal_expr() = default;
	literal_expr(const core::token& value, size_t line, size_t column)
		: value_(value), line_(line), column_(column) {
	}
};

struct variable_expr {
    core::token name_{};
    size_t line_{};
    size_t column_{};

	variable_expr() = default;
    variable_expr(const core::token& name, size_t line, size_t column)
        : name_(name), line_(line), column_(column) {}  
};

struct binary_expr {
    expr_ptr left_{};
    core::token op_{};
    expr_ptr right_{};
    size_t line_{};
    size_t column_{};

	binary_expr() = default;
	binary_expr(expr_ptr left, core::token op, expr_ptr right, size_t line, size_t column)
		: left_(std::move(left)), op_(op), right_(std::move(right)), line_(line), column_(column) {}
};

struct unary_expr {
    core::token op_{};
    expr_ptr operand_{};
    size_t line_{};
    size_t column_{};

	unary_expr() = default;
	unary_expr(core::token op, expr_ptr operand, size_t line, size_t column)
		: op_(op), operand_(std::move(operand)), line_(line), column_(column) {}
};

struct postfix_expr {
    expr_ptr operand_{};
	core::token op_{};
    size_t line_{};
    size_t column_{};

	postfix_expr() = default;
	postfix_expr(expr_ptr operand, core::token op, size_t line, size_t column)
		: operand_(std::move(operand)), op_(op), line_(line), column_(column) {}
};

struct call_expr {
    core::token callee_{};
    std::vector<expr_ptr> args_{};
    size_t line_{};
    size_t column_{};

	call_expr() = default;
	call_expr(const core::token& callee, std::vector<expr_ptr> args, 
              size_t line, size_t column)
		: callee_(callee), args_(std::move(args)), line_(line), column_(column) {
	}
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

    expression() : data_(literal_expr{}) {}

	template <typename T>
	expression(T e) : data_(std::move(e)) {}
};

} // namespace ast