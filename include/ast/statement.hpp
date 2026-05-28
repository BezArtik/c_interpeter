// ast/statement.hpp

// This file defines the various statement types in the AST, 
// such as variable declarations, 
// control flow statements, and function declarations.


#pragma once

#include "ast/expression.hpp"
#include <vector>
#include <variant>
#include <memory>

namespace ast {

struct statement;

using stmt_ptr = std::unique_ptr<statement>;

struct expression_stmt {
	expr_ptr expr_{};
    size_t line_{};
	size_t column_{};

	expression_stmt() = default;
	expression_stmt(expr_ptr e, size_t line, size_t column) 
        : expr_(std::move(e)), line_(line), column_(column) {}
};

struct var_declaration {
	core::value_type type_{};
	core::token name_{};
	expr_ptr initializer_{};

	var_declaration() = default;
	var_declaration(core::value_type t, const core::token& n, expr_ptr init)
		: type_(t), name_(n), initializer_(std::move(init)) {
	}
};

struct block_stmt {
	std::vector<stmt_ptr> statements_{};
	size_t line_{};
	size_t column_{};

	block_stmt() = default;
	block_stmt(size_t line, size_t column) : line_(line), column_(column) {}
};

struct while_stmt {
    expr_ptr condition_{};
    stmt_ptr body_{};
	size_t line_{};
	size_t column_{};

	while_stmt() = default;
	while_stmt(expr_ptr cond, stmt_ptr body, size_t line, size_t column)
		: condition_(std::move(cond)), body_(std::move(body)), line_(line), column_(column) {
	}
};

struct for_stmt {
	stmt_ptr initializer_{};
	expr_ptr condition_{};
	expr_ptr increment_{};
	stmt_ptr body_{};
	size_t line_{};
	size_t column_{};

	for_stmt() = default;
	for_stmt(stmt_ptr init, expr_ptr cond, expr_ptr inc, stmt_ptr body, size_t line, size_t column)
		: initializer_(std::move(init)), condition_(std::move(cond)), 
        increment_(std::move(inc)), body_(std::move(body)), line_(line), column_(column) {}
};

struct if_stmt {
    expr_ptr condition_{};
    stmt_ptr then_branch_{};
    stmt_ptr else_branch_{};
	size_t line_{};
	size_t column_{};

	if_stmt() = default;
	if_stmt(expr_ptr cond, stmt_ptr then_branch,
		    stmt_ptr else_branch, size_t line, size_t column)
		: condition_(std::move(cond)), then_branch_(std::move(then_branch)),
		else_branch_(std::move(else_branch)), line_(line), column_(column) {}
};

struct return_stmt {
	core::token keyword_{};
	expr_ptr value_{};

	return_stmt() = default;
	return_stmt(const core::token& kw, expr_ptr val)
		: keyword_(kw), value_(std::move(val)) {}
};

struct func_param {
    core::value_type type_{};
    core::token name_{};

	func_param() = default;
	func_param(core::value_type t, const core::token& n)
		: type_(t), name_(n) {
	}
};

struct func_declaration {
    core::value_type return_type_{};
    core::token name_{};
	std::vector<func_param> params_{};
	std::unique_ptr<block_stmt> body_{};

	func_declaration() = default;
	func_declaration(core::value_type ret_type, const core::token& n)
		: return_type_(ret_type), name_(n) {
	}
};

struct statement {
    std::variant<
        expression_stmt,
        var_declaration,
        block_stmt,
        for_stmt,
        while_stmt,
        if_stmt,
        return_stmt,
        func_declaration
    > data_;

	statement() : data_(expression_stmt{}) {}

	template <typename T>
	statement(T s) : data_(std::move(s)) {}
};

} // namespace ast